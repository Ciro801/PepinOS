[BITS 32]

EXTERN idt_handler_default
EXTERN idt_handler_div0
EXTERN idt_handler_pagefault
EXTERN idt_handler_gpf
EXTERN idt_handler_numbered
EXTERN do_switch
EXTERN dbg_irq0_iret
EXTERN keyboard_handler
EXTERN syscall_handler

GLOBAL _idt_default
GLOBAL _idt_div0
GLOBAL _idt_pagefault
GLOBAL _idt_gpf
GLOBAL _idt_irq0
GLOBAL _idt_irq1
GLOBAL _idt_syscall

; Excepciones SIN error code: push número, llama handler, haltea
%macro ISR_NOERR 1
GLOBAL _idt_isr %+ %1
_idt_isr %+ %1:
    pusha
    push dword %1
    call idt_handler_numbered
    add  esp, 4
    popa
    iret
%endmacro

; Excepciones CON error code: descarta error code, push número, haltea
%macro ISR_ERR 1
GLOBAL _idt_isr %+ %1
_idt_isr %+ %1:
    pop  eax                ; descartar error code (no lo necesitamos por ahora)
    pusha
    push dword %1
    call idt_handler_numbered
    add  esp, 4
    popa
    iret
%endmacro

; Excepciones sin error code
ISR_NOERR  1   ; Debug
ISR_NOERR  2   ; NMI
ISR_NOERR  3   ; Breakpoint
ISR_NOERR  4   ; Overflow
ISR_NOERR  5   ; Bound Range Exceeded
ISR_NOERR  6   ; Invalid Opcode
ISR_NOERR  7   ; Device Not Available (FPU)
ISR_NOERR  9   ; Coprocessor Segment Overrun
ISR_NOERR 15   ; Reserved
ISR_NOERR 16   ; x87 FPU Error
ISR_NOERR 18   ; Machine Check (no real error code en todos los casos)
ISR_NOERR 19   ; SIMD FP Exception
ISR_NOERR 20   ; Virtualization Exception

; Excepciones con error code
ISR_ERR    8   ; Double Fault
ISR_ERR   10   ; Invalid TSS
ISR_ERR   11   ; Segment Not Present
ISR_ERR   12   ; Stack Fault
ISR_ERR   17   ; Alignment Check

; IRQs de hardware no manejadas (34-47) — con EOI
%macro IRQ_DEFAULT 1
GLOBAL _idt_irq %+ %1
_idt_irq %+ %1:
    pusha
    push dword (32 + %1)
    call idt_handler_numbered
    add  esp, 4
    mov  al, 0x20
    out  0x20, al
    popa
    iret
%endmacro

IRQ_DEFAULT  2
IRQ_DEFAULT  3
IRQ_DEFAULT  4
IRQ_DEFAULT  5
IRQ_DEFAULT  6
IRQ_DEFAULT  7

_idt_default:
    pusha
    call idt_handler_default
    mov al, 0x20
    out 0x20, al
    popa
    iret

_idt_div0:
    pusha
    call idt_handler_div0
    popa
    iret

_idt_pagefault:
    pop  eax                ; eax = error code
    pusha                   ; salvar regs (8 × 4 = 32 bytes)
    mov  ebx, [esp + 32]    ; EIP faulting = esp+32 (justo encima del frame pusha)
    push ebx                ; 2do arg: faulting EIP
    push eax                ; 1er arg: error code
    call idt_handler_pagefault
    add  esp, 8
    popa
    iret

_idt_gpf:
    pop  eax
    pusha
    push eax
    call idt_handler_gpf
    add  esp, 4
    popa
    iret

_idt_irq0:
    pusha
    push esp              ; pasar puntero al frame pusha
    call do_switch        ; do_switch(ctx) guarda/restaura contexto
    add  esp, 4
    mov  al, 0x20
    out  0x20, al         ; EOI al PIC
    ; -- DIAGNÓSTICO: volcar frame iret antes de ejecutar iret --
    lea  eax, [esp + 32]  ; apunta al frame iret (justo encima del frame pusha)
    push eax
    call dbg_irq0_iret    ; dbg_irq0_iret(u32 *frame)
    add  esp, 4
    ; -----------------------------------------------------------
    popa
    iret

_idt_irq1:
    pusha
    call keyboard_handler
    mov al, 0x20
    out 0x20, al
    popa
    iret

_idt_syscall:
    push edx
    push ecx
    push ebx
    push eax
    call syscall_handler
    add  esp, 16
    iret
