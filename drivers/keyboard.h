#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#include "types.h"

#define KEYBOARD_PORT 0x60      /* puerto de datos del 8042 */

/* posición del cursor en pantalla */
extern char kX;
extern char kY;

void init_keyboard(void);
void keyboard_handler(void);

#endif