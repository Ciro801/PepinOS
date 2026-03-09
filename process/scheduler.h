#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "types.h"
#include "list.h"   /* struct list_head, container_of, list_for_each_entry */
#include "vfs.h"    /* fd_t, MAX_FD */
#include "signal.h" /* NSIG, SIGINT, SIGUSR1, SIGUSR2, SIGTERM, sighandler_t */

#define MAX_TASKS 8

/*
 * Contexto guardado en la pila durante IRQ0 (pusha + iret frame de ring 3):
 *   regs[0]  = EDI   \
 *   regs[1]  = ESI    |
 *   regs[2]  = EBP    | frame de pusha (8 registros)
 *   regs[3]  = ESP_   |  (ignorado por popa)
 *   regs[4]  = EBX    |
 *   regs[5]  = EDX    |
 *   regs[6]  = ECX    |
 *   regs[7]  = EAX   /
 *   regs[8]  = EIP   \
 *   regs[9]  = CS     | frame de iret (desde ring 3)
 *   regs[10] = EFLAGS |
 *   regs[11] = ESP    | (pila usuario real)
 *   regs[12] = SS    /
 */
typedef struct {
    u32              regs[13];           /* pusha(8) + iret(5)                    */
    u32              cr3;                /* page directory fisico                  */
    u32              kstack_top;         /* tope de la pila kernel (TSS.esp0)      */
    int              active;
    fd_t             files[MAX_FD];      /* tabla de descriptores de archivo       */
    struct list_head list;               /* enlace en la lista global de tareas    */

    /* ── Señales ── */
    u32              sig_pending;        /* bitmask de señales pendientes          */
    u32              sig_handlers[NSIG]; /* handler[signum] = addr usuario o 0     */
    u32              sig_saved_regs[13]; /* contexto guardado al entregar señal    */
    int              sig_in_handler;     /* 1 si estamos dentro de un handler      */
} task_t;

/* Lista global de tareas (cabeza centinela, definida en scheduler.c) */
extern struct list_head task_list;
extern int ntasks;

void    sched_add_task(u32 eip, u32 cs, u32 eflags,
                       u32 user_esp, u32 ss, u32 cr3, u32 kstack_top);
void    do_switch(u32 *ctx);
task_t *sched_current_task(void);
void    sched_signal(int signum);  /* marcar señal pendiente en la tarea actual */

#endif
