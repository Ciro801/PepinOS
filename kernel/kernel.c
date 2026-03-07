#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "paging.h"
#include "task.h"

extern char kY;
extern char kattr;

int kmain(void);

void _start(void)
{
    kY    = 0;
    kattr = 0x0A;
    print("PepinOS: iniciando...\n");

    init_gdt();

    asm("  movw $0x18, %ax  \n"
        "  movw %ax,  %ss   \n"
        "  movl $0x20000, %esp");

    kmain();
}

int kmain(void)
{
    kattr = 0x0F;
    print("================================\n");
    kattr = 0x0B;
    print(" PepinOS Paso 12 - Mem Separada\n");
    kattr = 0x0F;
    print("================================\n\n");

    kattr = 0x07;
    print("  [OK] GDT con ring 3 y TSS\n");

    init_idt();
    print("  [OK] IDT lista\n");

    init_keyboard();
    asm("sti");
    print("  [OK] Interrupciones activas\n");

    init_paging();
    print("  [OK] Paginacion kernel activa\n\n");

    print("Creando espacio usuario:\n");
    launch_task();

    while(1);
    return 0;
}