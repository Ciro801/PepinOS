/*
 * link_user.ld — Linker script para programas de usuario en PepinOS.
 *
 * El espacio de usuario empieza en 0x40000000 (directorio 256 de la PD).
 * El kernel mapea código y pila de cada proceso en esa misma dirección
 * virtual pero con page directories separados.
 */
ENTRY(_start)

SECTIONS
{
    . = 0x40000000;

    .text   : { *(.text)   }
    .rodata : { *(.rodata) }
    .data   : { *(.data)   }
    .bss    : { *(.bss)    }
}
