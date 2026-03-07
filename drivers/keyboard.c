#include "types.h"
#include "screen.h"
#include "keyboard.h"

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

static u8 inb(u16 port)
{
    u8 val;
    asm volatile ("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

void keyboard_handler(void)
{
    u8 scancode = inb(KEYBOARD_PORT);

    if (scancode & 0x80)
        return;

    if (scancode >= KBMAP_SIZE)
        return;

    char c = kbmap[scancode];

    if (c == 0)
        return;

    kattr = 0x0F;
    putcar(c);
}

void init_keyboard(void) { }