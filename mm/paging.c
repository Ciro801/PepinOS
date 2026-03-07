#include "types.h"
#include "paging.h"

/* limpiar un bloque de memoria */
static void memzero(u8 *ptr, u32 size)
{
    u32 i;
    for (i = 0; i < size; i++)
        ptr[i] = 0;
}

/* ============================================================
 * init_paging — configura el espacio del kernel
 *
 * Crea el Page Directory del kernel con identity mapping
 * para los primeros 16MB. Activa la paginación.
 * ============================================================ */
void init_paging(void)
{
    u32 *pd = (u32 *) KERNEL_PD_ADDR;
    u32 *pt = (u32 *) KERNEL_PT_ADDR;
    u32 i;

    /* limpiar Page Directory */
    memzero((u8 *) pd, 4096);

    /* identity mapping: 0x0 → 0x1000000 (16MB, 4 tablas) */
    u32 addr = 0;
    for (i = 0; i < 4 * 1024; i++) {
        pt[i] = addr | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
        addr += 0x1000;
    }

    /* llenar Page Directory con las 4 tablas */
    for (i = 0; i < 4; i++) {
        pd[i] = ((u32)(pt + i * 1024))
                | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    /* activar paginación */
    asm volatile ("mov %0, %%cr3" : : "r"(pd));
    u32 cr0;
    asm volatile ("mov %%cr0, %0" : "=r"(cr0));
    cr0 |= 0x80000000;
    asm volatile ("mov %0, %%cr0" : : "r"(cr0));
}

/* ============================================================
 * create_task_space — crea un espacio de memoria para una tarea
 *
 * La tarea ve:
 *   task_code_virt → código  (físico: task_code_phys)
 *   task_stack_virt → pila   (físico: task_stack_phys)
 *   KERNEL_VIRTUAL  → kernel (identity mapped)
 * ============================================================ */
void create_task_space(u32 task_code_phys, u32 task_stack_phys,
                       u32 task_code_virt, u32 task_stack_virt)
{
    u32 *pd = (u32 *) TASK_PD_ADDR;
    u32 *pt = (u32 *) TASK_PT_ADDR;
    u32 i;

    /* limpiar page directory y la page table de la tarea */
    memzero((u8 *) pd, 4096);
    memzero((u8 *) pt, 4096);

    /* ── Page table de la tarea: código y pila en el mismo directorio ──
     * TASK_CODE_VIRT = 0x40000000 → dir 256, página 0
     * TASK_STACK_VIRT = 0x40001000 → dir 256, página 1
     * Ambos caen en code_dir, así que van en la misma page table (pt).
     */
    u32 code_idx  = (task_code_virt  >> 12) & 0x3FF;
    u32 stack_idx = (task_stack_virt >> 12) & 0x3FF;
    u32 code_dir  =  task_code_virt  >> 22;

    pt[code_idx]  = task_code_phys  | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    pt[stack_idx] = task_stack_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    pd[code_dir] = ((u32) pt) | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;

    /* ── Kernel accesible via identidad (pd[0..3] del kernel) ──
     * El kernel está en 0x1000-0x2FFFF (físico = virtual con identidad).
     * Durante syscalls el CPU sigue con este CR3, así que necesita
     * acceder al código del kernel. Los entries del kernel ya tienen
     * PAGE_USER porque init_paging los creó así.
     */
    u32 *kernel_pd = (u32 *) KERNEL_PD_ADDR;
    for (i = 0; i < 4; i++)
        pd[i] = kernel_pd[i];   /* pd[0..3]: identidad 0-16MB */

    /* ── Kernel en zona alta (opcional, para future ring0 mapping) ── */
    u32 *kernel_pt = (u32 *) KERNEL_PT_ADDR;
    u32 kv_dir = KERNEL_VIRTUAL >> 22;   /* 0xC0000000 >> 22 = 768 */
    for (i = 0; i < 4; i++)
        pd[kv_dir + i] = ((u32)(kernel_pt + i * 1024))
                         | PAGE_PRESENT | PAGE_WRITE;
}

/* cambiar a espacio de la tarea */
void switch_to_task_space(void)
{
    asm volatile ("mov %0, %%cr3" : : "r"((u32)TASK_PD_ADDR));
}

/* volver al espacio del kernel */
void switch_to_kernel_space(void)
{
    asm volatile ("mov %0, %%cr3" : : "r"((u32)KERNEL_PD_ADDR));
}
