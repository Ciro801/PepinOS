#ifndef _SIGNAL_H_
#define _SIGNAL_H_

#include "types.h"

/*
 * signal.h — Constantes y tipos para señales POSIX de PepinOS.
 *
 * La entrega de señales ocurre en scheduler.c (do_switch), justo antes del
 * iret que restaura el contexto de usuario.  El mecanismo es:
 *
 *   1. do_switch detecta sig_pending != 0 && !sig_in_handler en la tarea.
 *   2. Construye un call frame en la pila de usuario:
 *        [ESP-4] = signum        (argumento al handler)
 *        [ESP-8] = 0             (dirección de retorno dummy)
 *   3. Redirige EIP al sig_handlers[signum] y activa sig_in_handler.
 *   4. El handler DEBE finalizar con sys_sigreturn() (syscall 10), que
 *      restaura el contexto guardado en sig_saved_regs.
 *
 * Señales soportadas:
 *   SIGINT  (2)  — Ctrl+C (teclado → sched_signal)
 *   SIGUSR1 (10) — Señal de usuario 1 (raise desde shell)
 *   SIGUSR2 (12) — Señal de usuario 2
 *   SIGTERM (15) — Terminación normal (raise desde shell)
 */

/* Número total de señales (bitmask de 32 bits → máximo 31 señales reales) */
#define NSIG    32

/* Señales estándar implementadas */
#define SIGINT   2   /* Interrupción de terminal (Ctrl+C)     */
#define SIGUSR1 10   /* Señal de usuario 1                    */
#define SIGUSR2 12   /* Señal de usuario 2                    */
#define SIGTERM 15   /* Terminación normal                    */

/*
 * Tipo de un manejador de señal en espacio de usuario.
 * El handler recibe el número de señal como argumento y DEBE terminar
 * con sys_sigreturn() — nunca con un 'return' ordinario.
 */
typedef void (*sighandler_t)(int signum);

#endif
