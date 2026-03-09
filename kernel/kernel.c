#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"
#include "ide.h"
#include "ext2.h"
#include "elf.h"
#include "task.h"

extern char kY;
extern char kattr;

/* Imprime un byte como dos digitos hexadecimales */
static void print_byte_hex(u8 b)
{
    char hex[] = "0123456789ABCDEF";
    char s[3];
    s[0] = hex[(b >> 4) & 0xF];
    s[1] = hex[b & 0xF];
    s[2] = '\0';
    print(s);
}

static void test_ide(void)
{
    u8 buf[512];
    int i;

    /* Escribir firma de prueba en el sector 0 del disco */
    for (i = 0; i < 512; i++) buf[i] = 0;
    buf[0] = 0xDE;
    buf[1] = 0xAD;
    buf[2] = 0xBE;
    buf[3] = 0xEF;
    ide_write_sector(0, buf);

    /* Leer de vuelta y verificar */
    for (i = 0; i < 512; i++) buf[i] = 0;
    ide_read_sector(0, buf);

    print("  [IDE] Sector 0 bytes [0-3]: ");
    print_byte_hex(buf[0]); print(" ");
    print_byte_hex(buf[1]); print(" ");
    print_byte_hex(buf[2]); print(" ");
    print_byte_hex(buf[3]); print("\n");

    if (buf[0] == 0xDE && buf[1] == 0xAD &&
        buf[2] == 0xBE && buf[3] == 0xEF) {
        print("  [OK] IDE: escritura y lectura correctas\n");
    } else {
        print("  [!!] IDE: fallo en verificacion\n");
    }
}

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
    print("  PepinOS Paso 19 - ELF Loader\n");
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
    print("  [OK] VMM: listo\n");

    test_ide();

    ext2_init();
    print("  [Ext2] Directorio raiz:\n");
    ext2_ls(EXT2_ROOT_INODE);
    {
        static u8 content[128];
        u32 ino = ext2_find(EXT2_ROOT_INODE, "hola.txt");
        if (ino) {
            u32 bytes = ext2_read_file(ino, content, 127);
            content[bytes] = '\0';
            print("  [Ext2] hola.txt: ");
            print((char *)content);
        }
    }
    print("\n");

    print("Iniciando multitarea:\n");
    launch_tasks();
    elf_exec("hello.elf");
    sched_enter();

    while(1);
    return 0;
}