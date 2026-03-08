#ifndef _VMM_H_
#define _VMM_H_

#include "types.h"
#include "paging.h"

/*
 * VMM — Virtual Memory Manager
 *
 * Gestiona el espacio virtual del kernel:
 *   - vmm_map_page   : mapea una dirección virtual a una física
 *   - vmm_unmap_page : desmapea una dirección virtual
 *
 * Si al mapear no existe aún la page table para ese directorio,
 * se asigna automáticamente una página física del PMM.
 *
 * Todas las operaciones operan sobre el page directory del kernel
 * (KERNEL_PD_ADDR) y son válidas sólo en modo ring 0.
 */

void vmm_init(void);

/* Mapea virt → phys con los flags dados (PAGE_PRESENT | PAGE_WRITE | ...) */
void vmm_map_page(u32 virt, u32 phys, u32 flags);

/* Desmapea virt e invalida la entrada TLB correspondiente */
void vmm_unmap_page(u32 virt);

#endif
