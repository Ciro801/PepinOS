#ifndef _TSS_H_
#define _TSS_H_

#include "types.h"

/* Estructura completa del TSS para i386 */
struct tss {
    u16 previous_task, reserved0;
    u32 esp0;           /* pila kernel (ring 0) */
    u16 ss0, reserved1; /* segmento pila kernel */
    u32 esp1;
    u16 ss1, reserved2;
    u32 esp2;
    u16 ss2, reserved3;
    u32 cr3;
    u32 eip, eflags;
    u32 eax, ecx, edx, ebx;
    u32 esp, ebp, esi, edi;
    u16 es,  reserved4;
    u16 cs,  reserved5;
    u16 ss,  reserved6;
    u16 ds,  reserved7;
    u16 fs,  reserved8;
    u16 gs,  reserved9;
    u16 ldt, reserved10;
    u16 trap, iomap_base;
} __attribute__((packed));

extern struct tss ktss;

void init_tss(u32 kernel_stack);

#endif