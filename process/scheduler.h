#ifndef _SCHEDULER_H_
#define _SCHEDULER_H_

#include "types.h"

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
    u32 regs[13];   /* pusha(8) + iret(5) */
    u32 cr3;        /* page directory fisico */
    int active;
} task_t;

extern int ntasks;

void sched_add_task(u32 eip, u32 cs, u32 eflags,
                    u32 user_esp, u32 ss, u32 cr3);
void do_switch(u32 *ctx);

#endif
