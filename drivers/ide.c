#include "ide.h"
#include "types.h"

/* Puertos del canal IDE primario */
#define IDE_DATA        0x1F0
#define IDE_SECCOUNT    0x1F2
#define IDE_LBA_LO      0x1F3
#define IDE_LBA_MID     0x1F4
#define IDE_LBA_HI      0x1F5
#define IDE_DRIVE_HEAD  0x1F6
#define IDE_STATUS      0x1F7   /* lectura */
#define IDE_COMMAND     0x1F7   /* escritura */

/* Bits del registro de estado */
#define IDE_SR_BSY      0x80    /* busy */
#define IDE_SR_DRQ      0x08    /* data request */
#define IDE_SR_ERR      0x01    /* error */

/* Comandos */
#define IDE_CMD_READ    0x20
#define IDE_CMD_WRITE   0x30
#define IDE_CMD_FLUSH   0xE7

static inline u8 inb(u16 port)
{
    u8 val;
    asm volatile("inb %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outb(u16 port, u8 val)
{
    asm volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}

static inline u16 inw(u16 port)
{
    u16 val;
    asm volatile("inw %1, %0" : "=a"(val) : "Nd"(port));
    return val;
}

static inline void outw(u16 port, u16 val)
{
    asm volatile("outw %0, %1" : : "a"(val), "Nd"(port));
}

/* Espera hasta que el disco no este ocupado (BSY=0) */
static void ide_wait_ready(void)
{
    while (inb(IDE_STATUS) & IDE_SR_BSY)
        ;
}

/* Espera hasta que haya datos disponibles (DRQ=1) */
static void ide_wait_drq(void)
{
    u8 st;
    do {
        st = inb(IDE_STATUS);
    } while ((st & IDE_SR_BSY) || !(st & IDE_SR_DRQ));
}

/*
 * ide_select_sector — configura los registros LBA y selecciona el drive
 * Drive 0 = master, modo LBA28
 */
static void ide_select_sector(u32 lba)
{
    ide_wait_ready();
    outb(IDE_DRIVE_HEAD, 0xE0 | ((lba >> 24) & 0x0F)); /* LBA mode, drive 0 */
    outb(IDE_SECCOUNT,   1);
    outb(IDE_LBA_LO,     (u8)(lba & 0xFF));
    outb(IDE_LBA_MID,    (u8)((lba >> 8) & 0xFF));
    outb(IDE_LBA_HI,     (u8)((lba >> 16) & 0xFF));
}

/*
 * ide_read_sector — lee 512 bytes del sector LBA en buf
 */
void ide_read_sector(u32 lba, u8 *buf)
{
    u16 *ptr = (u16 *) buf;
    int  i;

    ide_select_sector(lba);
    outb(IDE_COMMAND, IDE_CMD_READ);
    ide_wait_drq();

    for (i = 0; i < 256; i++)
        ptr[i] = inw(IDE_DATA);
}

/*
 * ide_write_sector — escribe 512 bytes de buf en el sector LBA
 */
void ide_write_sector(u32 lba, const u8 *buf)
{
    const u16 *ptr = (const u16 *) buf;
    int         i;

    ide_select_sector(lba);
    outb(IDE_COMMAND, IDE_CMD_WRITE);
    ide_wait_drq();

    for (i = 0; i < 256; i++)
        outw(IDE_DATA, ptr[i]);

    /* Forzar escritura al disco (flush cache) */
    outb(IDE_COMMAND, IDE_CMD_FLUSH);
    ide_wait_ready();
}
