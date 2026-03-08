#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"
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
        "  movl $0x120000, %esp");   /* pila kernel encima del binario */

    kmain();
}

int kmain(void)
{
    kattr = 0x0F;
    print("================================\n");
    kattr = 0x0B;
    print("  PepinOS Paso 16 - Gestion de Memoria\n");
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
    print("  [OK] Paginacion kernel activa\n");

    pmm_init(32 * 1024 * 1024);    /* 32MB de RAM */
    print("  [OK] PMM: memoria fisica lista\n");

    vmm_init();
    print("  [OK] VMM: listo\n\n");

    print("Iniciando multitarea:\n");
    launch_tasks();

    while(1);
    return 0;
}