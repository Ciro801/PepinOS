#ifndef _SCREEN_H_
#define _SCREEN_H_

#include "types.h"

extern char kX;
extern char kY;
extern char kattr;

void scrollup(unsigned int n);
void putcar(uchar c);
void print(char *string);
void update_cursor(void);

#endif