; multiboot.asm — Cabecera Multiboot (Especificación Multiboot 1)
;
; GRUB busca esta cabecera en los primeros 8KB del binario.
; Al encontrarla, activa modo protegido y salta al entry point (ELF)
; con EAX = 0x2BADB002 y EBX = puntero a info de boot.
;
; Flags 0x3 = bit0 (alinear módulos a página) + bit1 (info de memoria)

[BITS 32]

MULTIBOOT_MAGIC  equ 0x1BADB002
MULTIBOOT_FLAGS  equ 0x00000003
MULTIBOOT_CHECK  equ -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECK
