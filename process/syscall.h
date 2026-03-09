#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "types.h"

/* Números de syscall */
#define SYS_PRINT    0
#define SYS_EXIT     1
#define SYS_SETCOLOR 2
#define SYS_OPEN     3   /* ebx=path           → eax=fd (≥3) o -1 si error */
#define SYS_CLOSE    4   /* ebx=fd                                          */
#define SYS_READ     5   /* ebx=fd, ecx=buf, edx=count → eax=bytes leídos  */

/* Handler llamado desde el stub ASM — retorna int → EAX al proceso usuario */
int syscall_handler(u32 eax, u32 ebx, u32 ecx, u32 edx);

/* Handler ASM — registrado en IDT[0x30] */
extern void _idt_syscall(void);

#endif
