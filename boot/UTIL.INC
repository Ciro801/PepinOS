; =====================================================
; UTIL.INC — Funciones utilitarias compartidas
; =====================================================

; -----------------------------------------------------
; mostrar: imprime cadena terminada en 0 via BIOS
; Entrada: DS:SI apunta a la cadena
; -----------------------------------------------------
mostrar:
    push ax
    push bx
.bucle:
    lodsb
    cmp al, 0
    jz  .fin_mostrar
    mov ah, 0x0E
    mov bx, 0x07
    int 0x10
    jmp .bucle
.fin_mostrar:
    pop bx
    pop ax
    ret