#include "types.h"
#include "pmm.h"

/*
 * Bitmap de páginas físicas.
 * Índice i representa la página en la dirección física i * 4096.
 * Bit a 1 = ocupada, bit a 0 = libre.
 */
static u32 pmm_bitmap[PMM_MAX_PAGES / 32];   /* 256 palabras = 1KB */
static u32 pmm_total_pages = 0;
static u32 pmm_free_pages  = 0;

static void pmm_set_used(u32 idx)
{
    pmm_bitmap[idx / 32] |= (1u << (idx % 32));
}

static void pmm_set_free(u32 idx)
{
    pmm_bitmap[idx / 32] &= ~(1u << (idx % 32));
}

static int pmm_is_used(u32 idx)
{
    return (pmm_bitmap[idx / 32] >> (idx % 32)) & 1;
}

/*
 * pmm_init — inicializa el gestor de memoria física
 * mem_size: cantidad de RAM disponible en bytes
 *
 * Marca las primeras PMM_RESERVED (4MB) como ocupadas (kernel +
 * estructuras estáticas) y el resto como libre.
 */
void pmm_init(u32 mem_size)
{
    u32 i;

    pmm_total_pages = mem_size / PMM_PAGE_SIZE;
    if (pmm_total_pages > PMM_MAX_PAGES)
        pmm_total_pages = PMM_MAX_PAGES;

    pmm_free_pages = 0;

    /* marcar todas como ocupadas por defecto */
    for (i = 0; i < PMM_MAX_PAGES / 32; i++)
        pmm_bitmap[i] = 0xFFFFFFFF;

    /* liberar páginas por encima de PMM_RESERVED */
    for (i = PMM_RESERVED / PMM_PAGE_SIZE; i < pmm_total_pages; i++) {
        pmm_set_free(i);
        pmm_free_pages++;
    }
}

/*
 * pmm_alloc_page — asigna una página física libre
 * Devuelve la dirección física (alineada a 4KB) o 0 si no hay memoria.
 */
u32 pmm_alloc_page(void)
{
    u32 i;
    for (i = 0; i < pmm_total_pages; i++) {
        if (!pmm_is_used(i)) {
            pmm_set_used(i);
            pmm_free_pages--;
            return i * PMM_PAGE_SIZE;
        }
    }
    return 0;   /* sin memoria */
}

/*
 * pmm_free_page — libera una página física
 */
void pmm_free_page(u32 addr)
{
    u32 idx = addr / PMM_PAGE_SIZE;
    if (idx < pmm_total_pages && pmm_is_used(idx)) {
        pmm_set_free(idx);
        pmm_free_pages++;
    }
}

u32 pmm_free_count(void)
{
    return pmm_free_pages;
}
