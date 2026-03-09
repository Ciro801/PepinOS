/*
 * signal.c — Módulo de señales POSIX de PepinOS.
 *
 * Las señales en PepinOS se entregan en espacio de usuario mediante
 * inyección de frame en la pila.  No existe un módulo de entrega
 * independiente: toda la lógica vive en scheduler.c (do_switch) porque
 * la entrega ocurre justo antes del iret que restaura el contexto de
 * usuario, lo que garantiza que el handler arranque de forma limpia.
 *
 * ── Ciclo de vida de una señal ───────────────────────────────────────────
 *
 *  [Evento]                          [Función]
 *  Ctrl+C detectado por 8042    →   keyboard_handler()
 *                                       └→ sched_signal(SIGINT)
 *
 *  sys_kill (INT 0x30, eax=9)   →   syscall_handler()
 *                                       └→ sched_signal(signum)
 *
 *  sched_signal(signum)         →   scheduler.c
 *                                       └→ cur_task->sig_pending |= (1<<sig)
 *
 *  Timer IRQ0                   →   do_switch() en scheduler.c
 *                                       └→ detecta sig_pending
 *                                       └→ guarda regs en sig_saved_regs
 *                                       └→ inyecta call frame en pila usuario
 *                                       └→ redirige EIP al sig_handlers[sig]
 *
 *  Handler de usuario           →   user/shell.c  (sigint_handler, etc.)
 *                                       └→ hace su trabajo
 *                                       └→ llama sys_sigreturn()
 *
 *  sys_sigreturn (INT 0x30, eax=10) → syscall_handler()
 *                                       └→ restaura sig_saved_regs
 *                                       └→ limpia sig_in_handler
 *                                       └→ iret vuelve al punto original
 *
 * ── Registro de handlers ────────────────────────────────────────────────
 *
 *  sys_sigaction(signum, handler) → SYS_SIGACTION (eax=8) en syscall.c
 *                                       └→ cur_task->sig_handlers[sig] = addr
 *
 * ── Restricciones ───────────────────────────────────────────────────────
 *
 *  - No hay reentrada: sig_in_handler impide anidar handlers.
 *  - No hay señales bloqueadas (sin sigprocmask).
 *  - El handler debe usar sys_sigreturn() para retornar, no 'return'.
 *  - Sólo se entrega una señal por tick de timer.
 */

#include "signal.h"
