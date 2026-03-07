#include "types.h"
#include "tss.h"

struct tss ktss;

/*
 * init_tss — inicializa el TSS del kernel
 * kernel_stack: dirección de la pila kernel para cuando
 *               una interrupción llega desde ring 3
 */
void init_tss(u32 kernel_stack)
{
    /* limpiar toda la estructura */
    u8 *p = (u8 *) &ktss;
    u32 i;
    for (i = 0; i < sizeof(struct tss); i++)
        p[i] = 0;

    ktss.ss0  = 0x18;           /* selector pila kernel = GDT[3] */
    ktss.esp0 = kernel_stack;   /* pila kernel en ring 0 */
    ktss.iomap_base = sizeof(struct tss); /* sin mapa de puertos */
}