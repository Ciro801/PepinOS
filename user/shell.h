#ifndef _SHELL_H_
#define _SHELL_H_

/*
 * shell.h — Referencia de syscalls y señales para programas de usuario
 *           en PepinOS (espacio de usuario, ring 3).
 *
 * Los programas de usuario invocan el kernel mediante:
 *
 *   int $0x30  (trap gate IDT[0x30], DPL=3)
 *
 * Convención de llamada:
 *   EAX = número de syscall
 *   EBX = argumento 1
 *   ECX = argumento 2
 *   EDX = argumento 3
 *   Valor de retorno → EAX
 *
 * ── Números de syscall ───────────────────────────────────────────────────
 */
#define SYS_PRINT     0   /* ebx = const char *s                           */
#define SYS_EXIT      1   /* (sin args) — detiene el proceso               */
#define SYS_SETCOLOR  2   /* ebx = u8 attr  (color VGA: bg<<4 | fg)        */
#define SYS_OPEN      3   /* ebx = const char *path  → eax = fd (o -1)     */
#define SYS_CLOSE     4   /* ebx = int fd                                   */
#define SYS_READ      5   /* ebx = fd, ecx = buf, edx = count → eax = n    */
#define SYS_GETCHAR   6   /* (sin args) → eax = char (0 si vacío)          */
#define SYS_LS        7   /* lista el directorio raíz por pantalla          */
#define SYS_SIGACTION 8   /* ebx = signum, ecx = handler_addr               */
#define SYS_KILL      9   /* ebx = signum  — señal al proceso actual        */
#define SYS_SIGRETURN 10  /* restaura contexto previo a la señal            */

/*
 * ── Señales soportadas ───────────────────────────────────────────────────
 *
 * Los handlers se registran con SYS_SIGACTION y DEBEN finalizar con
 * SYS_SIGRETURN.  Usar 'return' normal corrompe el flujo de ejecución.
 */
#define SIGINT   2    /* Interrupción de terminal (Ctrl+C)   */
#define SIGUSR1 10    /* Señal de usuario 1                  */
#define SIGUSR2 12    /* Señal de usuario 2                  */
#define SIGTERM 15    /* Terminación normal                   */

/*
 * ── Colores VGA para SYS_SETCOLOR ───────────────────────────────────────
 *
 * attr = (fondo << 4) | frente
 * Ejemplo: 0x0F = fondo negro, texto blanco brillante
 *          0x0A = fondo negro, texto verde
 *          0x0C = fondo negro, texto rojo
 *          0x0E = fondo negro, texto amarillo
 *          0x0B = fondo negro, texto cian
 */

#endif
