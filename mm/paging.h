#ifndef _PAGING_H_
#define _PAGING_H_

#include "types.h"

/*
 * Mapa de memoria (Paso 15 — GRUB carga el kernel en 0x100000):
 *   0x100000 - 0x11FFFF : Kernel ELF (128KB reservados)
 *   0x120000            : Tope de la pila del kernel inicial
 *   0x200000            : Page Directory del kernel
 *   0x201000 - 0x204FFF : Page Tables del kernel (4 × 4KB = 16KB)
 *   0x205000 - 0x208FFF : Page Directories/Tables de las tareas
 *   0x300000+           : Código y pilas de las tareas de usuario
 *   0x320000+           : Pilas kernel por tarea (ring 0)
 */

/* Page Directory y Tables del KERNEL */
#define KERNEL_PD_ADDR   0x00200000
#define KERNEL_PT_ADDR   0x00201000    /* 4 tablas × 4KB = 16KB (hasta 0x204FFF) */

/* Page Directory y Tables de la TAREA A */
#define TASK_A_PD_ADDR   0x00205000
#define TASK_A_PT_ADDR   0x00206000

/* Page Directory y Tables de la TAREA B */
#define TASK_B_PD_ADDR   0x00207000
#define TASK_B_PT_ADDR   0x00208000

/* Flags */
#define PAGE_PRESENT     0x1
#define PAGE_WRITE       0x2
#define PAGE_USER        0x4

/* Dirección virtual donde el kernel se mapea en el espacio usuario */
#define KERNEL_VIRTUAL   0xC0000000

void init_paging(void);
void create_task_space(u32 task_code_phys, u32 task_stack_phys,
                       u32 task_code_virt, u32 task_stack_virt,
                       u32 pd_addr, u32 pt_addr);
void switch_to_kernel_space(void);

#endif
