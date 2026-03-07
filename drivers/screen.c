#include "types.h"

#define RAMSCREEN  0xB8000   /* inicio RAM de video */
#define SIZESCREEN 0xFA0     /* 4000 bytes = 25 filas × 80 cols × 2 */
#define SCREENLIM  0xB8FA0   /* fin RAM de video */

char kX    = 0;              /* columna actual del cursor */
char kY    = 0;              /* fila actual del cursor */
char kattr = 0x07;           /* atributo: blanco sobre negro */

/* escribir en puerto I/O */
static void outb(u16 port, u8 val)
{
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

/* mover el cursor hardware a la posición (kX, kY) */
void update_cursor(void)
{
    unsigned short pos = kY * 80 + kX;

    outb(0x3D4, 0x0F);              /* índice: byte bajo */
    outb(0x3D5, (u8)(pos & 0xFF));
    outb(0x3D4, 0x0E);              /* índice: byte alto */
    outb(0x3D5, (u8)((pos >> 8) & 0xFF));
}

/*
 * scrollup: desplaza la pantalla n líneas hacia arriba
 */
void scrollup(unsigned int n)
{
    unsigned char *video, *tmp;

    for (video = (unsigned char *) RAMSCREEN;
         video < (unsigned char *) SCREENLIM;
         video += 2)
    {
        tmp = (unsigned char *)(video + n * 160);

        if (tmp < (unsigned char *) SCREENLIM) {
            *video       = *tmp;
            *(video + 1) = *(tmp + 1);
        } else {
            *video       = 0;
            *(video + 1) = 0x07;
        }
    }

    kY -= n;
    if (kY < 0)
        kY = 0;
}

/*
 * putcar: imprime un carácter en la posición actual del cursor
 */
void putcar(uchar c)
{
    unsigned char *video;

    if (c == 10) {          /* \n — nueva línea */
        kX = 0;
        kY++;
    } else if (c == 9) {    /* \t — tabulación */
        kX = kX + 8 - (kX % 8);
    } else if (c == 13) {   /* \r — retorno de carro */
        kX = 0;
    } else {
        video  = (unsigned char *)(RAMSCREEN + 2 * kX + 160 * kY);
        *video       = c;
        *(video + 1) = kattr;
        kX++;
        if (kX > 79) {
            kX = 0;
            kY++;
        }
    }

    if (kY > 24)
        scrollup(kY - 24);

    update_cursor();
}

/*
 * print: imprime una cadena terminada en '\0'
 */
void print(char *string)
{
    while (*string != 0) {
        putcar(*string);
        string++;
    }
}
