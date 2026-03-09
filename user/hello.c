/*
 * hello.c — Primer programa de usuario ELF de PepinOS.
 *
 * Usa las syscalls del kernel via INT 0x30:
 *   SYS_PRINT   (EAX=0): imprime la string en EBX
 *   SYS_SETCOLOR(EAX=2): cambia el color de texto (EBX=atributo VGA)
 */

static void sys_print(const char *s)
{
    asm volatile("int $0x30" : : "a"(0), "b"(s));
}

static void sys_setcolor(unsigned char attr)
{
    asm volatile("int $0x30" : : "a"(2), "b"((unsigned int)attr));
}

void _start(void)
{
    /* TEST MINIMO: sin syscalls. Si fault persiste → bug en scheduler */
    while (1);
}
