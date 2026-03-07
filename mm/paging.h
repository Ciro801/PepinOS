#ifndef _PAGING_H_
#define _PAGING_H_

#include "types.h"

/* Page Directory y Tables del KERNEL */
#define KERNEL_PD_ADDR   0x00100000    /* 1MB — fuera del kernel */
#define KERNEL_PT_ADDR   0x00101000    /* justo después */

/* Page Directory y Tables de la TAREA USUARIO */
#define TASK_PD_ADDR     0x00110000
#define TASK_PT_ADDR     0x00111000

/* Flags */
#define PAGE_PRESENT     0x1
#define PAGE_WRITE       0x2
#define PAGE_USER        0x4

/* Dirección virtual donde el kernel se mapea en el espacio usuario */
#define KERNEL_VIRTUAL   0xC0000000

void init_paging(void);
void create_task_space(u32 task_code_phys, u32 task_stack_phys,
                       u32 task_code_virt, u32 task_stack_virt);
void switch_to_task_space(void);
void switch_to_kernel_space(void);

#endif