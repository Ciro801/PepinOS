#ifndef _GDT_H_
#define _GDT_H_

#include "types.h"

#define GDTSIZE  8              /* más descriptores para ring 3 y TSS */
#define GDTBASE  0x00000D00

struct gdtdesc {
    u16 lim0_15;
    u16 base0_15;
    u8  base16_23;
    u8  acces;
    u8  lim16_19 : 4;
    u8  other    : 4;
    u8  base24_31;
} __attribute__((packed));

struct gdtr {
    u16 limite;
    u32 base;
} __attribute__((packed));

#ifdef __GDT__
    struct gdtdesc kgdt[GDTSIZE];
    struct gdtr    kgdtr;
#else
    extern struct gdtdesc kgdt[GDTSIZE];
    extern struct gdtr    kgdtr;
#endif

/*
 * Selectores de segmento:
 * Bits 0-1 = RPL (nivel de privilegio del solicitante)
 * Bit  2   = TI  (0=GDT, 1=LDT)
 * Bits 3+  = índice en la GDT
 *
 * GDT[0] = NULL         → 0x00
 * GDT[1] = código  r0   → 0x08
 * GDT[2] = datos   r0   → 0x10
 * GDT[3] = pila    r0   → 0x18
 * GDT[4] = código  r3   → 0x20 | 3 = 0x23
 * GDT[5] = datos   r3   → 0x28 | 3 = 0x2B
 * GDT[6] = pila    r3   → 0x30 | 3 = 0x33
 * GDT[7] = TSS          → 0x38
 */

void init_gdt_desc(u32 base, u32 limite, u8 acces, u8 other,
                   struct gdtdesc *desc);
void init_gdt(void);

#endif