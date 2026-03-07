; =====================================================
; bootsect.asm — Bootloader con Modo Protegido
; =====================================================

%define BASE   0x100    ; kernel en dirección física 0x1000
%define KSIZE  50       ; más sectores por si el kernel crece

[BITS 16]
[ORG 0x0]

    jmp start
    %include "UTIL.INC"

start:
    ; --- Inicializar segmentos ---
    mov ax, 0x07C0
    mov ds, ax
    mov es, ax
    mov ax, 0x8000
    mov ss, ax
    mov sp, 0xF000

    ; --- Guardar unidad de boot ---
    mov [bootdrv], dl

    ; --- Mensaje de inicio ---
    mov si, msgCargando
    call mostrar

    ; --- Leer kernel del disco ---
    xor ax, ax
    int 0x13            ; resetear disco

    push es
    mov ax, BASE
    mov es, ax
    mov bx, 0
    mov ah, 0x02
    mov al, KSIZE
    mov ch, 0
    mov cl, 2
    mov dh, 0
    mov dl, [bootdrv]
    int 0x13
    pop es

    ; ===================================================
    ; ACTIVAR MODO PROTEGIDO
    ; ===================================================

    ; --- Paso 1: Calcular tamaño de la GDT ---
    mov ax, gdtfin
    mov bx, gdt
    sub ax, bx
    mov word [gdtptr], ax       ; límite de la GDT

    ; --- Paso 2: Calcular dirección física de la GDT ---
    ; dirección física = DS × 16 + offset_de_gdt
    xor eax, eax
    xor ebx, ebx
    mov ax, ds
    mov ecx, eax
    shl ecx, 4                  ; ecx = DS × 16
    mov bx, gdt
    add ecx, ebx                ; ecx = dirección física de gdt
    mov dword [gdtptr+2], ecx   ; base de la GDT

    ; --- Paso 3: Deshabilitar interrupciones ---
    cli

    ; --- Paso 4: Cargar la GDT en el registro GDTR ---
    lgdt [gdtptr]

    ; --- Paso 5: Activar modo protegido (bit PE del CR0) ---
    mov eax, cr0
    or  ax, 1
    mov cr0, eax

    ; --- Paso 6: Far jump para limpiar pipeline del CPU ---
    jmp siguiente
siguiente:

    ; --- Paso 7: Recargar selectores de segmento de datos ---
    mov ax, 0x10        ; selector GDT[2] = segmento de datos
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x9F000    ; nueva pila en modo protegido

    ; --- Paso 8: Far jump al kernel (recarga CS con GDT[1]) ---
    jmp dword 0x08:0x1000

; ===================================================
; DATOS
; ===================================================
msgCargando db "PepinOS: activando modo protegido...", 13, 10, 0
bootdrv     db 0

; ===================================================
; GDT — Global Descriptor Table
; Cada entrada: 8 bytes
; ===================================================
gdt:
    ; Descriptor 0: NULL (obligatorio)
    db 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

    ; Descriptor 1: Segmento de CÓDIGO (selector 0x08)
    ; base=0x0, límite=0xFFFFF, 32-bit, ring 0, ejecutable
    db 0xFF, 0xFF           ; límite bits 0-15
    db 0x00, 0x00, 0x00     ; base bits 0-23
    db 10011011b            ; acceso: presente, ring0, código, ejecutable, legible
    db 11011111b            ; flags: 32-bit, granularidad 4KB + límite bits 16-19
    db 0x00                 ; base bits 24-31

    ; Descriptor 2: Segmento de DATOS (selector 0x10)
    ; base=0x0, límite=0xFFFFF, 32-bit, ring 0, escritura
    db 0xFF, 0xFF           ; límite bits 0-15
    db 0x00, 0x00, 0x00     ; base bits 0-23
    db 10010011b            ; acceso: presente, ring0, datos, escribible
    db 11011111b            ; flags: 32-bit, granularidad 4KB + límite bits 16-19
    db 0x00                 ; base bits 24-31
gdtfin:

; Puntero GDTR (6 bytes: 2 límite + 4 base)
gdtptr:
    dw 0    ; límite (calculado en runtime)
    dd 0    ; base   (calculada en runtime)

; --- Relleno y firma MBR ---
times 510-($-$$) db 0x90
dw 0xAA55