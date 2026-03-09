/*
 * shell.c — Shell interactivo de PepinOS con soporte de señales POSIX.
 *
 * Syscalls (via INT 0x30):
 *   0  SYS_PRINT    ebx=string
 *   1  SYS_EXIT
 *   2  SYS_SETCOLOR ebx=attr
 *   3  SYS_OPEN     ebx=path   -> eax=fd
 *   4  SYS_CLOSE    ebx=fd
 *   5  SYS_READ     ebx=fd, ecx=buf, edx=count -> eax=bytes
 *   6  SYS_GETCHAR  -> eax=char (0 si vacio)
 *   7  SYS_LS       lista directorio raiz
 *   8  SYS_SIGACTION ebx=signum, ecx=handler_addr
 *   9  SYS_KILL     ebx=signum -> senal al proceso actual
 *  10  SYS_SIGRETURN -> restaura contexto pre-senal
 */

#define SIGINT   2
#define SIGTERM 15
#define SIGUSR1 10
#define SIGUSR2 12

/* -- Syscall wrappers ---------------------------------------------------- */

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

static int sys_getchar(void)
{
    int c;
    asm volatile("int $0x30" : "=a"(c) : "a"(6));
    return c;
}

static void sys_ls(void)
{
    asm volatile("int $0x30" : : "a"(7));
}

static void sys_sigaction(int sig, void *handler)
{
    asm volatile("int $0x30" : : "a"(8), "b"(sig), "c"((unsigned int)handler));
}

static void sys_kill(int sig)
{
    asm volatile("int $0x30" : : "a"(9), "b"(sig));
}

/* sys_sigreturn: restaura el contexto antes de la senal.
 * El kernel modifica el iret frame; no retorna al codigo del handler. */
static void sys_sigreturn(void)
{
    asm volatile("int $0x30" : : "a"(10));
}

/* -- Utilidades de string ------------------------------------------------- */

static int str_eq(const char *a, const char *b)
{
    while (*a && *b && *a == *b) { a++; b++; }
    return *a == *b;
}

static int str_starts(const char *s, const char *prefix)
{
    while (*prefix) {
        if (*s++ != *prefix++) return 0;
    }
    return 1;
}

static const char *str_skip_word(const char *s)
{
    while (*s && *s != ' ') s++;
    while (*s == ' ') s++;
    return s;
}

/* Convierte string decimal en entero (solo dígitos, sin signo) */
static int str_atoi(const char *s)
{
    int n = 0;
    while (*s >= '0' && *s <= '9') { n = n * 10 + (*s - '0'); s++; }
    return n;
}

/* -- Manejadores de señal ------------------------------------------------- */

/*
 * Los manejadores deben terminar con sys_sigreturn().
 * NO usar 'return' — el kernel redirige el EIP al llamar sigreturn.
 */

static void sigint_handler(int sig)
{
    (void)sig;
    sys_setcolor(0x0C);
    sys_print("\n[SIGINT] Ctrl+C recibido. Escribe 'help' para ver comandos.\n");
    sys_sigreturn();
}

static void sigusr1_handler(int sig)
{
    (void)sig;
    sys_setcolor(0x0E);
    sys_print("[SIGUSR1] Senal de usuario 1 recibida.\n");
    sys_sigreturn();
}

static void sigterm_handler(int sig)
{
    (void)sig;
    sys_setcolor(0x0C);
    sys_print("[SIGTERM] Terminando...\n");
    /* En un OS real aqui se llamaria exit. Por ahora colgamos el proceso. */
    while (1) { sys_getchar(); }
}

/* -- Comandos ------------------------------------------------------------- */

static void cmd_help(void)
{
    sys_setcolor(0x0B);
    sys_print("Comandos disponibles:\n");
    sys_setcolor(0x07);
    sys_print("  help          - muestra esta ayuda\n");
    sys_print("  ls            - lista el directorio raiz\n");
    sys_print("  cat <archivo> - muestra el contenido de un archivo\n");
    sys_print("  echo <texto>  - imprime texto\n");
    sys_print("  clear         - limpia la pantalla\n");
    sys_print("  raise <N>     - envia la senal N al proceso actual\n");
    sys_print("\nSenales instaladas:\n");
    sys_print("  SIGINT  (2)   - Ctrl+C\n");
    sys_print("  SIGUSR1 (10)  - raise 10\n");
    sys_print("  SIGTERM (15)  - raise 15\n");
}

static void cmd_cat(const char *filename)
{
    static char buf[512];
    int fd, n;

    if (!*filename) {
        sys_setcolor(0x0C);
        sys_print("uso: cat <archivo>\n");
        return;
    }

    fd = sys_open(filename);
    if (fd < 0) {
        sys_setcolor(0x0C);
        sys_print("cat: no encontrado: ");
        sys_print(filename);
        sys_print("\n");
        return;
    }

    sys_setcolor(0x0F);
    n = sys_read(fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        sys_print(buf);
        if (buf[n - 1] != '\n') sys_print("\n");
    }
    sys_close(fd);
}

static void cmd_echo(const char *text)
{
    sys_setcolor(0x0F);
    sys_print(text);
    sys_print("\n");
}

static void cmd_clear(void)
{
    int i;
    sys_setcolor(0x00);
    for (i = 0; i < 25; i++) sys_print("\n");
    sys_setcolor(0x0F);
}

static void cmd_raise(const char *arg)
{
    int sig;
    if (!*arg) {
        sys_setcolor(0x0C);
        sys_print("uso: raise <numero_senal>\n");
        return;
    }
    sig = str_atoi(arg);
    if (sig <= 0) {
        sys_setcolor(0x0C);
        sys_print("raise: numero de senal invalido\n");
        return;
    }
    sys_setcolor(0x07);
    sys_print("Enviando senal ");
    sys_print(arg);
    sys_print("...\n");
    sys_kill(sig);
    /* La senal se entregara en el proximo timer tick */
}

/* -- Dispatch ------------------------------------------------------------- */

static void execute(const char *line)
{
    if (str_eq(line, "help")) {
        cmd_help();
    } else if (str_eq(line, "ls")) {
        sys_ls();
    } else if (str_starts(line, "cat ")) {
        cmd_cat(str_skip_word(line));
    } else if (str_eq(line, "cat")) {
        cmd_cat("");
    } else if (str_starts(line, "echo ")) {
        cmd_echo(str_skip_word(line));
    } else if (str_eq(line, "echo")) {
        sys_print("\n");
    } else if (str_eq(line, "clear")) {
        cmd_clear();
    } else if (str_starts(line, "raise ")) {
        cmd_raise(str_skip_word(line));
    } else if (str_eq(line, "raise")) {
        cmd_raise("");
    } else if (*line == '\0') {
        /* linea vacia */
    } else {
        sys_setcolor(0x0C);
        sys_print("desconocido: ");
        sys_print(line);
        sys_print("\n");
    }
}

/* -- Entry point ---------------------------------------------------------- */

void _start(void)
{
    static char line[128];
    int pos, c;

    /* Instalar manejadores de senal */
    sys_sigaction(SIGINT,  sigint_handler);
    sys_sigaction(SIGUSR1, sigusr1_handler);
    sys_sigaction(SIGTERM, sigterm_handler);

    /* Banner de bienvenida */
    sys_setcolor(0x0B);
    sys_print("\n  PepinOS Shell  (paso 24 - senales POSIX)\n");
    sys_setcolor(0x07);
    sys_print("  help: comandos disponibles\n");
    sys_print("  Ctrl+C envia SIGINT | 'raise N' envia la senal N\n\n");

    while (1) {
        /* Prompt */
        sys_setcolor(0x0A);
        sys_print("pepinos$ ");
        sys_setcolor(0x0F);

        pos = 0;

        /* Leer linea caracter a caracter */
        while (1) {
            do { c = sys_getchar(); } while (c == 0);

            if (c == '\n' || c == '\r') {
                line[pos] = '\0';
                break;
            } else if (c == '\b' || c == 127) {
                if (pos > 0) {
                    pos--;
                    sys_print("\b \b");
                }
            } else if (pos < (int)(sizeof(line) - 1)) {
                line[pos++] = (char)c;
            }
        }

        sys_print("\n");
        sys_setcolor(0x07);
        execute(line);
    }
}
