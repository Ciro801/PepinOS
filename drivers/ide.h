#ifndef _IDE_H_
#define _IDE_H_

#include "types.h"

/*
 * IDE — Driver de disco en modo PIO (Programmed I/O)
 *
 * Controla el canal IDE primario (puertos 0x1F0-0x1F7).
 * Usa direccionamiento LBA28: hasta 128GB con 28 bits.
 *
 * Cada sector tiene 512 bytes = 256 palabras de 16 bits.
 *
 * Protocolo basico:
 *   1. Seleccionar drive y LBA[27:24] en 0x1F6
 *   2. Enviar cuenta de sectores (0x1F2) y LBA[23:0] (0x1F3-5)
 *   3. Enviar comando READ(0x20) o WRITE(0x30) en 0x1F7
 *   4. Esperar BSY=0, DRQ=1 en el registro de estado (0x1F7)
 *   5. Leer/escribir 256 words desde/hacia 0x1F0
 */

/* Lee un sector de 512 bytes desde la LBA indicada */
void ide_read_sector(u32 lba, u8 *buf);

/* Escribe un sector de 512 bytes en la LBA indicada */
void ide_write_sector(u32 lba, const u8 *buf);

#endif
