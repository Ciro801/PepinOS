/*
 * hello.c — Primer programa de usuario ELF de PepinOS.
 *
 * Usa las syscalls del kernel via INT 0x30:
 *   SYS_PRINT   (0): imprime string en EBX
 *   SYS_SETCOLOR(2): cambia color de texto
 *   SYS_OPEN    (3): abre archivo → fd
 *   SYS_CLOSE   (4): cierra fd
 *   SYS_READ    (5): lee bytes de fd → cuenta
 */

static void sys_print(const char *s)
{
    asm volatile("int $0x30" : : "a"(0), "b"(s));
}

static void sys_setcolor(unsigned char attr)
{
    asm volatile("int $0x30" : : "a"(2), "b"((unsigned int)attr));
}

static int sys_open(const char *path)
{
    int fd;
    asm volatile("int $0x30" : "=a"(fd) : "a"(3), "b"(path));
    return fd;
}

static void sys_close(int fd)
{
    asm volatile("int $0x30" : : "a"(4), "b"(fd));
}

static int sys_read(int fd, char *buf, unsigned int count)
{
    int n;
    asm volatile("int $0x30" : "=a"(n) : "a"(5), "b"(fd), "c"(buf), "d"(count));
    return n;
}

void _start(void)
{
    char buf[64];
    int  fd, n;

    sys_setcolor(0x0A);   /* verde brillante */
    sys_print("hola desde PepinOS!\n");

    /* Leer hola.txt del disco via VFS */
    fd = sys_open("hola.txt");
    if (fd >= 0) {
        n = sys_read(fd, buf, 63);
        if (n > 0) {
            buf[n] = '\0';
            sys_setcolor(0x0E);   /* amarillo */
            sys_print("hola.txt: ");
            sys_print(buf);
        }
        sys_close(fd);
    }

    sys_setcolor(0x0A);
    while (1) {
        sys_print(".");
    }
}
