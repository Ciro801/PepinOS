#include "vmm.h"
#include "pmm.h"
#include "paging.h"
#include "types.h"

/*
 * vmm_init — no necesita hacer nada especial:
 * init_paging() ya configuro el page directory del kernel y activo CR0.PG.
 * pmm_init() debe haberse llamado antes para que pmm_alloc_page funcione.
 */
void vmm_init(void)
{
    /* nop — preparacion completa en init_paging + pmm_init */
}

/*
 * vmm_map_page — mapea virt -> phys en el page directory del kernel.
 *
 * Si la page table del directorio correspondiente aun no existe,
 * se asigna automaticamente una pagina fisica con pmm_alloc_page().
 * Como el kernel esta identity-mapped, la direccion fisica == virtual
 * y se puede escribir directamente sobre ella.
 */
void vmm_map_page(u32 virt, u32 phys, u32 flags)
{
    u32 pd_idx = virt >> 22;
    u32 pt_idx = (virt >> 12) & 0x3FF;
    u32 *pd    = (u32 *) KERNEL_PD_ADDR;
    u32 *pt;
    u32  i;

    if (!(pd[pd_idx] & PAGE_PRESENT)) {
        /* Asignar nueva page table desde el PMM */
        u32 pt_phys = pmm_alloc_page();
        pt = (u32 *) pt_phys;       /* identity mapped: virt == phys */
        for (i = 0; i < 1024; i++)
            pt[i] = 0;
        pd[pd_idx] = pt_phys | PAGE_PRESENT | PAGE_WRITE;
    }

    pt = (u32 *)(pd[pd_idx] & ~0xFFF);
    pt[pt_idx] = phys | flags;

    /* Invalidar la entrada TLB para esta direccion virtual */
    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}

/*
 * vmm_unmap_page — elimina el mapeo de virt e invalida el TLB.
 */
void vmm_unmap_page(u32 virt)
{
    u32 pd_idx = virt >> 22;
    u32 pt_idx = (virt >> 12) & 0x3FF;
    u32 *pd    = (u32 *) KERNEL_PD_ADDR;
    u32 *pt;

    if (!(pd[pd_idx] & PAGE_PRESENT))
        return;

    pt = (u32 *)(pd[pd_idx] & ~0xFFF);
    pt[pt_idx] = 0;

    asm volatile("invlpg (%0)" : : "r"(virt) : "memory");
}
