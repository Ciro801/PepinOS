#ifndef _TASK_H_
#define _TASK_H_

#include "types.h"

/* Direcciones FÍSICAS del código y pila de la tarea */
#define TASK_CODE_PHYS   0x00030000
#define TASK_STACK_PHYS  0x0003F000

/* Direcciones VIRTUALES que ve la tarea */
#define TASK_CODE_VIRT   0x40000000   /* código en 1GB virtual (dir 256) */
#define TASK_STACK_VIRT  0x40001000   /* pila justo después (dir 256, pag 1) */

void launch_task(void);
void user_task(void);

#endif