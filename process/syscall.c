#include "types.h"
#include "screen.h"
#include "syscall.h"

/*
 * syscall_handler — despachador de syscalls
 * Llamado desde _idt_syscall en idt_asm.asm
 *
 * eax = número de syscall
 * ebx = arg1, ecx = arg2, edx = arg3
 */
void syscall_handler(u32 eax, u32 ebx, u32 ecx, u32 edx)
{
    (void)ecx;
    (void)edx;

    switch (eax) {

    case SYS_PRINT:
        /* ebx = puntero a string */
        kattr = 0x0F;
        print((char *) ebx);
        break;

    case SYS_EXIT:
        kattr = 0x0C;
        print("\n[kernel] tarea terminada.\n");
        while (1);          /* por ahora detener el sistema */
        break;

    case SYS_SETCOLOR:
        /* ebx = nuevo atributo de color */
        kattr = (char) ebx;
        break;

    default:
        kattr = 0x0C;
        print("\n[kernel] syscall desconocida: ");
        /* imprimir número en decimal simple */
        putcar('0' + (eax % 10));
        putcar('\n');
        break;
    }
}