#ifndef _SYSCALL_H_
#define _SYSCALL_H_

#include "types.h"

/* Números de syscall */
#define SYS_PRINT     0
#define SYS_EXIT      1
#define SYS_SETCOLOR  2
#define SYS_OPEN      3   /* ebx=path           → eax=fd (≥3) o -1 si error */
#define SYS_CLOSE     4   /* ebx=fd                                          */
#define SYS_READ      5   /* ebx=fd, ecx=buf, edx=count → eax=bytes leídos  */
#define SYS_GETCHAR   6   /* → eax=char (0 si ring buffer vacío)             */
#define SYS_LS        7   /* Lista directorio raíz en pantalla               */
#define SYS_SIGACTION 8   /* ebx=signum, ecx=handler_addr                    */
#define SYS_KILL      9   /* ebx=signum → señal al proceso actual            */
#define SYS_SIGRETURN 10  /* restaura contexto pre-señal (llamar en handler) */

/*
 * Handler llamado desde el stub ASM — retorna int → EAX al proceso usuario.
 *
 * frame: puntero al bloque {eax,ebx,ecx,edx} + iret frame en pila kernel.
 *   frame[0] = eax  (reg usuario)        frame[4] = EIP
 *   frame[1] = ebx  (reg usuario)        frame[5] = CS
 *   frame[2] = ecx  (reg usuario)        frame[6] = EFLAGS
 *   frame[3] = edx  (reg usuario)        frame[7] = ESP_user
 *                                         frame[8] = SS
 * SYS_SIGRETURN lo modifica para restaurar el contexto pre-señal.
 */
int syscall_handler(u32 *frame, u32 eax, u32 ebx, u32 ecx, u32 edx);

/* Handler ASM — registrado en IDT[0x30] */
extern void _idt_syscall(void);

#endif
