#ifndef _TASK_H_
#define _TASK_H_

/* Direcciones FÍSICAS del código y pila de cada tarea (por encima del kernel) */
#define TASK_A_CODE_PHYS  0x00300000
#define TASK_A_STACK_PHYS 0x0030F000

#define TASK_B_CODE_PHYS  0x00301000
#define TASK_B_STACK_PHYS 0x0030E000

/* Direcciones VIRTUALES (iguales para ambas tareas — cada una tiene su PD) */
#define TASK_CODE_VIRT    0x40000000   /* dir 256, página 0 */
#define TASK_STACK_VIRT   0x40001000   /* dir 256, página 1 */

/* Pilas kernel por tarea (ring 0): 4KB cada una, identity-mapped */
#define TASK_A_KSTACK_TOP 0x00321000   /* pila A: 0x320000-0x320FFF */
#define TASK_B_KSTACK_TOP 0x00323000   /* pila B: 0x322000-0x322FFF */

void launch_tasks(void);
void user_task(void);
void user_task_b(void);

#endif
