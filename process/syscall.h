#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "types.h"

/* Números de syscall */
#define SYS_PRINT  0
#define SYS_EXIT   1
#define SYS_SETCOLOR 2

/* Handler llamado desde el stub ASM */
void syscall_handler(u32 eax, u32 ebx, u32 ecx, u32 edx);

/* Handler ASM — registrado en IDT[0x30] */
extern void _idt_syscall(void);

#endif