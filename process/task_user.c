/*
 * task_user.c — corre en ring 3, dirección virtual 0x0
 * Solo puede comunicarse con el kernel via INT 0x30
 */

#define SYSCALL0(num) \
    asm volatile("int $0x30" : : "a"(num))

#define SYSCALL1(num, arg1) \
    asm volatile("int $0x30" : : "a"(num), "b"(arg1))

void user_task(void)
{
    SYSCALL1(2, 0x0B);  /* color cian */
    SYSCALL1(0, (unsigned int)"================================\n");
    SYSCALL1(2, 0x0E);  /* amarillo */
    SYSCALL1(0, (unsigned int)"  Ring 3 - Espacio separado!   \n");
    SYSCALL1(2, 0x0F);
    SYSCALL1(0, (unsigned int)"================================\n\n");

    SYSCALL1(2, 0x07);
    SYSCALL1(0, (unsigned int)"Memoria virtual de esta tarea:\n");
    SYSCALL1(0, (unsigned int)"  0x0000 = mi codigo\n");
    SYSCALL1(0, (unsigned int)"  0x1000 = mi pila\n");
    SYSCALL1(0, (unsigned int)"  0xC000 = kernel (no accesible)\n\n");

    SYSCALL1(2, 0x0A);
    SYSCALL1(0, (unsigned int)"No puedo leer memoria del kernel.\n");
    SYSCALL1(0, (unsigned int)"Solo puedo hablar via syscalls.\n");

    SYSCALL1(2, 0x0E);
    SYSCALL1(0, (unsigned int)"\nEscribe algo (teclado activo):\n");
    SYSCALL1(2, 0x0F);

    SYSCALL0(1);    /* exit */

    while(1);
}