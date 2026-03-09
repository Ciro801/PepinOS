#include "screen.h"
#include "gdt.h"
#include "idt.h"
#include "keyboard.h"
#include "paging.h"
#include "pmm.h"
#include "vmm.h"
#include "ide.h"
#include "ext2.h"
#include "vfs.h"
#include "elf.h"
#include "task.h"

extern char kY;
extern char kattr;

/*
 * mbr_find_partition — lee el MBR y devuelve el LBA de inicio
 * de la primera partición válida (tipo 0x83 = Linux ext2).
 * Si no hay tabla de particiones (sin firma 0x55AA), devuelve 0
 * para mantener compatibilidad con el disco plano de desarrollo.
 */
static u32 mbr_find_partition(void)
{
    u8 mbr[512];
    u8 *entry;
    u32 lba;
    int i;

    ide_read_sector(0, mbr);

    /* Sin firma de MBR válida → disco plano (sin particiones) */
    if (mbr[510] != 0x55 || mbr[511] != 0xAA)
        return 0;

    /* Tabla de particiones: 4 entradas × 16 bytes desde offset 0x1BE */
    for (i = 0; i < 4; i++) {
        entry = mbr + 0x1BE + i * 16;
        if (entry[4] == 0x83) {           /* tipo Linux ext2/3/4 */
            lba = (u32)entry[8]        |
                  (u32)entry[9]  << 8  |
                  (u32)entry[10] << 16 |
                  (u32)entry[11] << 24;
            return lba;
        }
    }
    return 0;
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
    u32 part_lba;

    kattr = 0x0F;
    print("================================\n");
    kattr = 0x0B;
    print("  PepinOS Paso 20 - GRUB Boot\n");
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

    /* Leer la tabla de particiones del MBR para localizar el FS ext2 */
    part_lba = mbr_find_partition();
    ext2_set_partition(part_lba);
    ext2_init();
    vfs_init();
    print("  [OK] VFS: listo\n");

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
