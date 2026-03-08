#ifndef _PMM_H_
#define _PMM_H_

#include "types.h"

/*
 * PMM — Physical Memory Manager
 *
 * Gestiona las páginas físicas de 4KB mediante un bitmap.
 * Un bit = 1 página:  0 = libre,  1 = ocupada.
 *
 * Las primeras 4MB (0x000000-0x3FFFFF) se marcan siempre como
 * ocupadas (kernel, pilas, page tables).  Todo lo demás queda
 * disponible para asignación dinámica.
 */

#define PMM_PAGE_SIZE   4096
#define PMM_MAX_PAGES   8192        /* cubre 32MB = 8192 páginas */
#define PMM_RESERVED    0x400000    /* primeros 4MB reservados al arranque */

void pmm_init(u32 mem_size);

u32  pmm_alloc_page(void);          /* devuelve dirección física o 0 si OOM */
void pmm_free_page(u32 addr);

u32  pmm_free_count(void);          /* páginas libres actuales */

#endif
