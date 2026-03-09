#include "scheduler.h"
#include "tss.h"
#include "screen.h"

static void phex(u32 v)
{
    int j;
    for (j = 28; j >= 0; j -= 4) {
        u8 n = (v >> j) & 0xF;
        putcar(n < 10 ? '0' + n : 'A' + n - 10);
    }
}

static u32 dbg_switches = 0;
static u32 dbg_iret_count = 0;

/* Vuelca el frame iret sobre la pila kernel justo antes de que iret lo consuma */
void dbg_irq0_iret(u32 *frame)
{
    if (dbg_iret_count++ < 5) {
        print("[iret] eip="); phex(frame[0]);
        print(" cs=");        phex(frame[1]);
        print(" fl=");        phex(frame[2]);
        print(" esp=");       phex(frame[3]);
        print(" ss=");        phex(frame[4]);
        putcar('\n');
    }
}

static task_t tasks[MAX_TASKS];
int ntasks = 0;
static int current = 0;

void sched_add_task(u32 eip, u32 cs, u32 eflags,
                    u32 user_esp, u32 ss, u32 cr3, u32 kstack_top)
{
    task_t *t = &tasks[ntasks];
    u32 i;

    for (i = 0; i < 13; i++)
        t->regs[i] = 0;

    t->regs[8]   = eip;
    t->regs[9]   = cs;
    t->regs[10]  = eflags;
    t->regs[11]  = user_esp;
    t->regs[12]  = ss;
    t->cr3       = cr3;
    t->kstack_top = kstack_top;
    t->active    = 1;
    ntasks++;
}

/*
 * do_switch — cambia de tarea (llamado desde _idt_irq0)
 *
 * Si la interrupción llegó desde ring 0 (CS=0x08, durante una syscall),
 * no se cambia de tarea — se espera a que la tarea vuelva a ring 3.
 * Esto evita corrupción de pila al interrumpir código de kernel.
 *
 * Si llegó desde ring 3, guarda el contexto actual, selecciona la
 * siguiente tarea (round-robin), restaura su contexto y actualiza
 * TSS.esp0 con la pila kernel de la nueva tarea.
 */
void do_switch(u32 *ctx)
{
    task_t *cur = &tasks[current];
    task_t *next;
    u32 i;

    /* Si estamos en ring 0 (syscall en curso): no cambiar de tarea */
    if ((ctx[9] & 0x3) == 0)
        return;

    /* guardar contexto actual (ring 3) */
    for (i = 0; i < 13; i++)
        cur->regs[i] = ctx[i];
    asm volatile("mov %%cr3, %0" : "=r"(cur->cr3));

    /* siguiente tarea (round-robin) */
    current = (current + 1) % ntasks;
    next = &tasks[current];

    /* traza (solo primeros 20 cambios) */
    if (dbg_switches < 20) {
        dbg_switches++;
        print("[sw->"); phex(current);
        print(" eip="); phex(next->regs[8]);
        print(" esp="); phex(next->regs[11]);
        print("]\n");
    }

    /* restaurar contexto siguiente */
    for (i = 0; i < 13; i++)
        ctx[i] = next->regs[i];
    asm volatile("mov %0, %%cr3" : : "r"(next->cr3));

    /* actualizar TSS con la pila kernel de la siguiente tarea */
    ktss.esp0 = next->kstack_top;
}
