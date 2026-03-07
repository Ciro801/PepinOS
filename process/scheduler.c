#include "scheduler.h"

static task_t tasks[MAX_TASKS];
int ntasks = 0;
static int current = 0;

/*
 * sched_add_task — registra una tarea en el scheduler
 * El contexto inicial (regs[0..7] = pusha frame) se pone a 0.
 * El iret frame se inicializa con los valores pasados.
 */
void sched_add_task(u32 eip, u32 cs, u32 eflags,
                    u32 user_esp, u32 ss, u32 cr3)
{
    task_t *t = &tasks[ntasks];
    u32 i;

    for (i = 0; i < 13; i++)
        t->regs[i] = 0;

    t->regs[8]  = eip;
    t->regs[9]  = cs;
    t->regs[10] = eflags;
    t->regs[11] = user_esp;
    t->regs[12] = ss;
    t->cr3      = cr3;
    t->active   = 1;
    ntasks++;
}

/*
 * do_switch — cambia de tarea (llamado desde _idt_irq0)
 *
 * ctx apunta al frame pusha en la pila:
 *   ctx[0..7]  = registros guardados por pusha (EDI..EAX)
 *   ctx[8..12] = frame iret: EIP, CS, EFLAGS, user_ESP, user_SS
 *
 * Guarda el contexto de la tarea actual, selecciona la siguiente
 * (round-robin) y escribe su contexto en la pila para que el
 * iret la restaure automáticamente.
 */
void do_switch(u32 *ctx)
{
    task_t *cur  = &tasks[current];
    task_t *next;
    u32 i;

    /* guardar contexto actual */
    for (i = 0; i < 13; i++)
        cur->regs[i] = ctx[i];
    asm volatile("mov %%cr3, %0" : "=r"(cur->cr3));

    /* siguiente tarea (round-robin) */
    current = (current + 1) % ntasks;
    next = &tasks[current];

    /* restaurar contexto siguiente */
    for (i = 0; i < 13; i++)
        ctx[i] = next->regs[i];
    asm volatile("mov %0, %%cr3" : : "r"(next->cr3));
}
