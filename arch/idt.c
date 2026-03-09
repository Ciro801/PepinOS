#include "types.h"
#include "screen.h"
#include "syscall.h"

#define __IDT__
#include "idt.h"

#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1

static void outb(u16 port, u8 val)
{
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void init_idt_desc(u16 select, u32 offset, u16 type,
                   struct idtdesc *desc)
{
    desc->offset0_15  = (offset & 0x0000FFFF);
    desc->select      = select;
    desc->type        = type;
    desc->offset16_31 = (offset & 0xFFFF0000) >> 16;
}

static void print_hex(u32 val)
{
    int j;
    for (j = 28; j >= 0; j -= 4) {
        u8 nibble = (val >> j) & 0xF;
        putcar(nibble < 10 ? '0' + nibble : 'A' + nibble - 10);
    }
}

void idt_handler_pagefault(u32 error, u32 eip)
{
    u32 faulting_addr, cr3;
    asm volatile ("mov %%cr2, %0" : "=r"(faulting_addr));
    asm volatile ("mov %%cr3, %0" : "=r"(cr3));

    kattr = 0x0C;
    print("\n[!] PAGE FAULT addr=0x");
    print_hex(faulting_addr);
    print(" eip=0x");
    print_hex(eip);
    print(" err=0x");
    print_hex(error);
    print(error & 4 ? " [USER]" : " [KERN]");
    print("\n    cr3=0x");
    print_hex(cr3);
    putcar('\n');
    while (1);
}

void idt_handler_default(void)
{
    kattr = 0x0C;
    print("\n[!] Interrupcion no manejada\n");
}

void idt_handler_gpf(u32 error_code)
{
    kattr = 0x0C;
    print("\n[!] GENERAL PROTECTION FAULT  error_code: 0x");
    print_hex(error_code);
    putcar('\n');
    while (1);
}

/* Handler diagnóstico: imprime el número de excepción y haltea */
void idt_handler_numbered(u32 n)
{
    kattr = 0x0C;
    print("\n[!] EXCEPCION #");
    if (n >= 10) putcar('0' + n / 10);
    putcar('0' + n % 10);
    putcar('\n');
    while (1);
}

void idt_handler_div0(void)
{
    kattr = 0x0C;
    print("\n[!] EXCEPCION: Division por cero\n");
    while(1);
}

void init_idt(void)
{
    int i;

    /* Reprogramar 8259A */
    outb(PIC_MASTER_CMD,  0x11);
    outb(PIC_MASTER_DATA, 0x20);
    outb(PIC_MASTER_DATA, 0x04);
    outb(PIC_MASTER_DATA, 0x01);

    outb(PIC_SLAVE_CMD,   0x11);
    outb(PIC_SLAVE_DATA,  0x28);
    outb(PIC_SLAVE_DATA,  0x02);
    outb(PIC_SLAVE_DATA,  0x01);

    /* IRQ0 (timer) e IRQ1 (teclado) habilitados */
    outb(PIC_MASTER_DATA, 0xFC);    /* 11111100 */
    outb(PIC_SLAVE_DATA,  0xFF);

    /* Llenar IDT con handler por defecto */
    for (i = 0; i < IDTSIZE; i++)
        init_idt_desc(0x08, (u32)_idt_default, 0x8E00, &kidt[i]);

    /* Excepciones sin error code */
    init_idt_desc(0x08, (u32)_idt_div0,      0x8E00, &kidt[0]);
    init_idt_desc(0x08, (u32)_idt_isr1,      0x8E00, &kidt[1]);
    init_idt_desc(0x08, (u32)_idt_isr2,      0x8E00, &kidt[2]);
    init_idt_desc(0x08, (u32)_idt_isr3,      0x8E00, &kidt[3]);
    init_idt_desc(0x08, (u32)_idt_isr4,      0x8E00, &kidt[4]);
    init_idt_desc(0x08, (u32)_idt_isr5,      0x8E00, &kidt[5]);
    init_idt_desc(0x08, (u32)_idt_isr6,      0x8E00, &kidt[6]);
    init_idt_desc(0x08, (u32)_idt_isr7,      0x8E00, &kidt[7]);
    init_idt_desc(0x08, (u32)_idt_isr9,      0x8E00, &kidt[9]);
    init_idt_desc(0x08, (u32)_idt_isr15,     0x8E00, &kidt[15]);
    init_idt_desc(0x08, (u32)_idt_isr16,     0x8E00, &kidt[16]);
    init_idt_desc(0x08, (u32)_idt_isr19,     0x8E00, &kidt[19]);
    init_idt_desc(0x08, (u32)_idt_isr20,     0x8E00, &kidt[20]);

    /* Excepciones con error code */
    init_idt_desc(0x08, (u32)_idt_isr8,      0x8E00, &kidt[8]);
    init_idt_desc(0x08, (u32)_idt_isr10,     0x8E00, &kidt[10]);
    init_idt_desc(0x08, (u32)_idt_isr11,     0x8E00, &kidt[11]);
    init_idt_desc(0x08, (u32)_idt_isr12,     0x8E00, &kidt[12]);
    init_idt_desc(0x08, (u32)_idt_gpf,       0x8E00, &kidt[13]);
    init_idt_desc(0x08, (u32)_idt_pagefault, 0x8E00, &kidt[14]);
    init_idt_desc(0x08, (u32)_idt_isr17,     0x8E00, &kidt[17]);

    /* IRQs */
    init_idt_desc(0x08, (u32)_idt_irq0,      0x8E00, &kidt[32]);
    init_idt_desc(0x08, (u32)_idt_irq1,      0x8E00, &kidt[33]);
    init_idt_desc(0x08, (u32)_idt_irq2,      0x8E00, &kidt[34]);
    init_idt_desc(0x08, (u32)_idt_irq3,      0x8E00, &kidt[35]);
    init_idt_desc(0x08, (u32)_idt_irq4,      0x8E00, &kidt[36]);
    init_idt_desc(0x08, (u32)_idt_irq5,      0x8E00, &kidt[37]);
    init_idt_desc(0x08, (u32)_idt_irq6,      0x8E00, &kidt[38]);
    init_idt_desc(0x08, (u32)_idt_irq7,      0x8E00, &kidt[39]);

    /* Syscall — Trap Gate DPL=3 (mantiene IF=1, timer puede interrumpir) */
    init_idt_desc(0x08, (u32)_idt_syscall, 0xEF00, &kidt[0x30]);

    /* Cargar IDTR */
    kidtr.limite = IDTSIZE * sizeof(struct idtdesc);
    kidtr.base   = IDTBASE;

    u8 *src = (u8 *) kidt;
    u8 *dst = (u8 *) IDTBASE;
    u32 j;
    for (j = 0; j < kidtr.limite; j++)
        dst[j] = src[j];

    asm("lidtl (%0)" : : "r"(&kidtr));
}
