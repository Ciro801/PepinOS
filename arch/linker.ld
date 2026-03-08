/*
 * linker.ld — Script de enlace para PepinOS con GRUB/Multiboot
 *
 * El kernel se carga en 0x100000 (1MB), que es la dirección estándar
 * para kernels Multiboot. La sección .multiboot va primero para
 * que GRUB encuentre la cabecera en los primeros 8KB.
 */

ENTRY(_start)

SECTIONS {
    . = 0x100000;

    .multiboot ALIGN(4) : { *(.multiboot) }
    .text       ALIGN(4) : { *(.text)      }
    .rodata     ALIGN(4) : { *(.rodata)    }
    .data       ALIGN(4) : { *(.data)      }
    .bss        ALIGN(4) : { *(COMMON) *(.bss) }
}
