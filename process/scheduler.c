#include "scheduler.h"
#include "tss.h"
#include "screen.h"

/*
 * Lista circular de tareas.
 * task_list es la cabeza centinela (no representa ninguna tarea real).
 * Las tareas se insertan con list_add_tail → orden FIFO para round-robin.
 */
LIST_HEAD(task_list);

static task_t  task_pool[MAX_TASKS];  /* pool estático de task_t             */
int            ntasks   = 0;
static task_t *cur_task = 0;          /* tarea actualmente en ejecución       */

task_t *sched_current_task(void)
{
    return cur_task;
}

/* Marca la señal 'signum' como pendiente en la tarea actualmente en ejecución */
void sched_signal(int signum)
{
    if (!cur_task || signum <= 0 || signum >= NSIG) return;
    cur_task->sig_pending |= (1u << signum);
}

void sched_add_task(u32 eip, u32 cs, u32 eflags,
                    u32 user_esp, u32 ss, u32 cr3, u32 kstack_top)
{
    task_t *t = &task_pool[ntasks];
    u32 i;

    for (i = 0; i < 13; i++) t->regs[i] = 0;
    t->regs[8]    = eip;
    t->regs[9]    = cs;
    t->regs[10]   = eflags;
    t->regs[11]   = user_esp;
    t->regs[12]   = ss;
    t->cr3        = cr3;
    t->kstack_top = kstack_top;
    t->active     = 1;

    for (i = 0; i < MAX_FD; i++) {
        t->files[i].node   = 0;
        t->files[i].offset = 0;
        t->files[i].used   = 0;
    }

    /* Inicializar señales */
    t->sig_pending    = 0;
    t->sig_in_handler = 0;
    for (i = 0; i < NSIG; i++) t->sig_handlers[i] = 0;
    for (i = 0; i < 13;   i++) t->sig_saved_regs[i] = 0;

    /* Enlazar al final de la lista circular (FIFO = round-robin correcto) */
    list_add_tail(&task_list, &t->list);

    /* La primera tarea registrada sera la inicial */
    if (ntasks == 0) cur_task = t;
    ntasks++;
}

/*
 * do_switch — cambia de tarea (llamado desde _idt_irq0)
 *
 * Si la interrupción llegó desde ring 0 (CS=0x08), no cambiar de tarea.
 * En ring 3: guarda el contexto actual, avanza al siguiente nodo de la
 * lista circular y restaura su contexto.
 *
 * Al final, si la tarea siguiente tiene señales pendientes y no está ya
 * en un handler, inyecta la llamada al handler en su pila de usuario.
 */
void do_switch(u32 *ctx)
{
    struct list_head *next_node;
    task_t           *next;
    u32               i, sig;

    if (!cur_task) return;

    /* Si estamos en ring 0 (syscall en curso): no cambiar de tarea */
    if ((ctx[9] & 0x3) == 0) return;

    /* Guardar contexto de la tarea actual */
    for (i = 0; i < 13; i++)
        cur_task->regs[i] = ctx[i];
    asm volatile("mov %%cr3, %0" : "=r"(cur_task->cr3));

    /* Avanzar al siguiente nodo circular.
     * Si llegamos a la cabeza centinela (task_list), saltarla. */
    next_node = cur_task->list.next;
    if (next_node == &task_list)
        next_node = next_node->next;

    next     = list_entry(next_node, task_t, list);
    cur_task = next;

    /* Restaurar contexto de la siguiente tarea */
    for (i = 0; i < 13; i++)
        ctx[i] = next->regs[i];
    asm volatile("mov %0, %%cr3" : : "r"(next->cr3));

    /* Actualizar TSS con la pila kernel de la siguiente tarea */
    ktss.esp0 = next->kstack_top;

    /* ── Entrega de señales ─────────────────────────────────────────────────
     * Condiciones para entregar:
     *   - Hay señales pendientes
     *   - No estamos ya dentro de un handler (sin reentrada)
     *
     * Mecanismo de inyección (x86 cdecl):
     *   Construimos un call frame en la pila usuario antes del iret:
     *     [ESP-4] = signum      (arg1 de la función handler)
     *     [ESP-8] = 0           (dirección de retorno dummy)
     *   Luego redirigimos EIP al handler y ajustamos ESP.
     *   El handler DEBE terminar con sys_sigreturn(), no con return.
     * ──────────────────────────────────────────────────────────────────────*/
    if (next->sig_pending && !next->sig_in_handler) {
        for (sig = 1; sig < NSIG; sig++) {
            if (next->sig_pending & (1u << sig)) break;
        }

        if (sig < NSIG && next->sig_handlers[sig]) {
            /* Guardar contexto completo para sigreturn */
            for (i = 0; i < 13; i++)
                next->sig_saved_regs[i] = ctx[i];

            /* ctx[11] = ESP_user. Empujar call frame de usuario. */
            ctx[11] -= 4;
            *((u32 *) ctx[11]) = sig;   /* arg1: signum                 */
            ctx[11] -= 4;
            *((u32 *) ctx[11]) = 0;     /* ret addr dummy (no usar ret) */

            ctx[8] = next->sig_handlers[sig];  /* EIP → handler         */
            next->sig_in_handler = 1;
            next->sig_pending &= ~(1u << sig);
        }
    }
}
