#include "string.h"

/*
 * string.c — Implementación de utilidades de cadenas y memoria.
 *
 * Todas las funciones operan byte a byte para garantizar compatibilidad
 * con el entorno bare-metal (sin SSE/MMX, sin libc).
 *
 * Prefijo 'k' (kernel) para evitar conflictos si en el futuro se enlaza
 * con una mini-libc de usuario.
 */

u32 kstrlen(const char *s)
{
    u32 n = 0;
    while (s[n]) n++;
    return n;
}

int kstrcmp(const char *s, const char *t)
{
    while (*s && *t && *s == *t) { s++; t++; }
    return (unsigned char)*s - (unsigned char)*t;
}

int kstrncmp(const char *s, const char *t, u32 n)
{
    while (n && *s && *t && *s == *t) { s++; t++; n--; }
    if (n == 0) return 0;
    return (unsigned char)*s - (unsigned char)*t;
}

char *kstrcpy(char *dst, const char *src)
{
    char *d = dst;
    while ((*d++ = *src++));
    return dst;
}

void *kmemset(void *dst, int c, u32 n)
{
    unsigned char *p = (unsigned char *)dst;
    while (n--) *p++ = (unsigned char)c;
    return dst;
}

void *kmemcpy(void *dst, const void *src, u32 n)
{
    unsigned char       *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    while (n--) *d++ = *s++;
    return dst;
}
