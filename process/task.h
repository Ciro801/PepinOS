#ifndef _TASK_H_
#define _TASK_H_

/* Direcciones FÍSICAS del código y pila de cada tarea */
#define TASK_A_CODE_PHYS  0x00030000
#define TASK_A_STACK_PHYS 0x0003F000

#define TASK_B_CODE_PHYS  0x00031000
#define TASK_B_STACK_PHYS 0x0003E000

/* Direcciones VIRTUALES (iguales para ambas tareas — cada una tiene su PD) */
#define TASK_CODE_VIRT    0x40000000   /* dir 256, página 0 */
#define TASK_STACK_VIRT   0x40001000   /* dir 256, página 1 */

void launch_tasks(void);
void user_task(void);
void user_task_b(void);

#endif
