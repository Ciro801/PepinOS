#ifndef _IDT_H_
#define _IDT_H_

#include "types.h"

#define IDTSIZE  256
#define IDTBASE  0x00000500

struct idtdesc {
    u16 offset0_15;
    u16 select;
    u16 type;
    u16 offset16_31;
} __attribute__((packed));

struct idtr {
    u16 limite;
    u32 base;
} __attribute__((packed));

#ifdef __IDT__
    struct idtdesc kidt[IDTSIZE];
    struct idtr    kidtr;
#else
    extern struct idtdesc kidt[IDTSIZE];
    extern struct idtr    kidtr;
#endif

void init_idt_desc(u16 select, u32 offset, u16 type,
                   struct idtdesc *desc);
void init_idt(void);

/* handlers ASM */
extern void _idt_default(void);
extern void _idt_div0(void);
extern void _idt_irq0(void);
extern void _idt_irq1(void);
extern void _idt_irq2(void);
extern void _idt_irq3(void);
extern void _idt_irq4(void);
extern void _idt_irq5(void);
extern void _idt_irq6(void);
extern void _idt_irq7(void);
extern void _idt_syscall(void);
extern void _idt_pagefault(void);
extern void _idt_gpf(void);
/* stubs numerados para diagnóstico */
extern void _idt_isr1(void);
extern void _idt_isr2(void);
extern void _idt_isr3(void);
extern void _idt_isr4(void);
extern void _idt_isr5(void);
extern void _idt_isr6(void);
extern void _idt_isr7(void);
extern void _idt_isr8(void);
extern void _idt_isr9(void);
extern void _idt_isr10(void);
extern void _idt_isr11(void);
extern void _idt_isr12(void);
extern void _idt_isr15(void);
extern void _idt_isr16(void);
extern void _idt_isr17(void);
extern void _idt_isr19(void);
extern void _idt_isr20(void);

#endif