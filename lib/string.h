#ifndef _STRING_H_
#define _STRING_H_

#include "types.h"

/*
 * string.h — Utilidades de cadenas y memoria para el kernel de PepinOS.
 *
 * Implementaciones simples byte a byte, sin depender de la libc ni de
 * instrucciones SIMD (el kernel compila con -mno-sse -mno-mmx).
 */

/* Longitud de una cadena terminada en '\0' */
u32  kstrlen(const char *s);

/* Comparación lexicográfica de dos cadenas.
 * Retorna 0 si son iguales, <0 si s < t, >0 si s > t. */
int  kstrcmp(const char *s, const char *t);

/* Comparación de los primeros n caracteres. */
int  kstrncmp(const char *s, const char *t, u32 n);

/* Copia src en dst (incluyendo el '\0'). Retorna dst. */
char *kstrcpy(char *dst, const char *src);

/* Rellena n bytes de dst con el byte c. Retorna dst. */
void *kmemset(void *dst, int c, u32 n);

/* Copia n bytes de src a dst (sin solapamiento). Retorna dst. */
void *kmemcpy(void *dst, const void *src, u32 n);

#endif
