#include "types.h"
#include "tss.h"

#define __GDT__
#include "gdt.h"

void init_gdt_desc(u32 base, u32 limite, u8 acces, u8 other,
                   struct gdtdesc *desc)
{
    desc->lim0_15   = (limite & 0x0000FFFF);
    desc->base0_15  = (base   & 0x0000FFFF);
    desc->base16_23 = (base   & 0x00FF0000) >> 16;
    desc->acces     = acces;
    desc->lim16_19  = (limite & 0x000F0000) >> 16;
    desc->other     = (other  & 0x0F);
    desc->base24_31 = (base   & 0xFF000000) >> 24;
}

void init_gdt(void)
{
    /* GDT[0] NULL */
    init_gdt_desc(0x0, 0x0, 0x00, 0x00, &kgdt[0]);

    /* GDT[1] código kernel ring 0 — selector 0x08 */
    init_gdt_desc(0x0, 0xFFFFF, 0x9B, 0x0D, &kgdt[1]);

    /* GDT[2] datos kernel ring 0 — selector 0x10 */
    init_gdt_desc(0x0, 0xFFFFF, 0x93, 0x0D, &kgdt[2]);

    /* GDT[3] pila kernel ring 0 — selector 0x18 */
    init_gdt_desc(0x0, 0x0, 0x97, 0x0D, &kgdt[3]);

    /* GDT[4] código usuario ring 3 — selector 0x23 (0x20 | RPL=3) */
    init_gdt_desc(0x0, 0xFFFFF, 0xFF, 0x0D, &kgdt[4]);

    /* GDT[5] datos usuario ring 3 — selector 0x2B (0x28 | RPL=3) */
    init_gdt_desc(0x0, 0xFFFFF, 0xF3, 0x0D, &kgdt[5]);

    /* GDT[6] pila usuario ring 3 — selector 0x33 (0x30 | RPL=3) */
    init_gdt_desc(0x0, 0x0, 0xF7, 0x0D, &kgdt[6]);

    /* GDT[7] TSS — selector 0x38
     * acces=0x89: presente, ring0, tipo TSS disponible (1001b)
     * other=0x00: límite en bytes, 32-bit
     */
    init_gdt_desc((u32)&ktss, sizeof(struct tss),
                  0x89, 0x00, &kgdt[7]);

    /* Instalar GDT */
    kgdtr.limite = GDTSIZE * sizeof(struct gdtdesc);
    kgdtr.base   = GDTBASE;

    u8 *src = (u8 *) kgdt;
    u8 *dst = (u8 *) GDTBASE;
    u32 i;
    for (i = 0; i < kgdtr.limite; i++)
        dst[i] = src[i];

    asm("lgdtl (%0)" : : "r"(&kgdtr));

    asm("  movw $0x10, %ax  \n"
        "  movw %ax,  %ds   \n"
        "  movw %ax,  %es   \n"
        "  movw %ax,  %fs   \n"
        "  movw %ax,  %gs   \n"
        "  ljmp $0x08, $gdt_next \n"
        "gdt_next:           \n");
}