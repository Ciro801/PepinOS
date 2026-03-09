#include "types.h"
#include "screen.h"
#include "keyboard.h"
#include "scheduler.h"   /* sched_signal */

/* ── Ring buffer de teclado ─────────────────────────────────────────────── */
#define KB_BUF_SIZE 64
static char kb_buf[KB_BUF_SIZE];
static int  kb_head = 0;   /* posición de escritura */
static int  kb_tail = 0;   /* posición de lectura   */

/* Devuelve el siguiente carácter, o 0 si el buffer está vacío */
char kb_getchar(void)
{
    char c;
    if (kb_head == kb_tail) return 0;
    c = kb_buf[kb_tail];
    kb_tail = (kb_tail + 1) % KB_BUF_SIZE;
    return c;
}

static char kbmap[] = {
/*00*/  0,    0,   '1', '2', '3', '4', '5', '6',
/*08*/ '7',  '8', '9', '0', '-', '=',  0,    0,
/*10*/ 'q',  'w', 'e', 'r', 't', 'y', 'u', 'i',
/*18*/ 'o',  'p', '[', ']', '\n', 0,  'a', 's',
/*20*/ 'd',  'f', 'g', 'h', 'j', 'k', 'l', ';',
/*28*/ '\'', '`',  0,  '\\','z', 'x', 'c', 'v',
/*30*/ 'b',  'n', 'm', ',', '.', '/',  0,   '*',
/*38*/  0,   ' ',  0,   0,   0,   0,   0,   0,
/*40*/  0,    0,   0,   0,   0,   0,   0,  '7',
/*48*/ '8',  '9', '-', '4', '5', '6', '+', '1',
/*50*/ '2',  '3', '0', '.',  0,   0,   0,   0
};

#define KBMAP_SIZE (sizeof(kbmap) / sizeof(kbmap[0]))

/* Scancodes especiales */
#define SC_CTRL_PRESS   0x1D
#define SC_CTRL_RELEASE 0x9D

static u8 inb(u16 port)
{
    u8 val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void keyboard_handler(void)
{
    static int ctrl_held = 0;
    u8   scancode = inb(KEYBOARD_PORT);
    char c;
    int  next;

    /* Tecla Ctrl (izquierda) */
    if (scancode == SC_CTRL_PRESS)  { ctrl_held = 1; return; }
    if (scancode == SC_CTRL_RELEASE){ ctrl_held = 0; return; }

    /* Tecla liberada (bit 7 encendido) — ignorar */
    if (scancode & 0x80) return;

    if (scancode >= KBMAP_SIZE) return;

    c = kbmap[scancode];
    if (c == 0) return;

    /* Ctrl+C → SIGINT al proceso actual */
    if (ctrl_held && c == 'c') {
        kattr = 0x0C;
        putcar('^');
        putcar('C');
        putcar('\n');
        sched_signal(SIGINT);
        return;
    }

    /* Encolar en el ring buffer para sys_getchar */
    next = (kb_head + 1) % KB_BUF_SIZE;
    if (next != kb_tail) {
        kb_buf[kb_head] = c;
        kb_head = next;
    }

    /* Eco en pantalla */
    kattr = 0x0F;
    putcar(c);
}

void init_keyboard(void) { }
