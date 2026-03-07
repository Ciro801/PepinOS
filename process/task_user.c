/*
 * task_user.c — tareas de usuario (ring 3)
 * Se comunican con el kernel únicamente via INT 0x30 (syscalls).
 * El scheduler round-robin alterna entre ellas cada tick del timer.
 */

#define SYSCALL0(num) \
    asm volatile("int $0x30" : : "a"(num))

#define SYSCALL1(num, arg1) \
    asm volatile("int $0x30" : : "a"(num), "b"(arg1))

void user_task(void)
{
    volatile int i;
    SYSCALL1(2, 0x0B);      /* color cian */
    while (1) {
        SYSCALL1(0, (unsigned int)"[A]");
        for (i = 0; i < 1000000; i++);
    }
}

void user_task_b(void)
{
    volatile int i;
    SYSCALL1(2, 0x0E);      /* color amarillo */
    while (1) {
        SYSCALL1(0, (unsigned int)"[B]");
        for (i = 0; i < 1000000; i++);
    }
}
