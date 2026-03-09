#ifndef _ELF_H_
#define _ELF_H_

#include "types.h"

/*
 * Formato ELF32 — solo los campos necesarios para el cargador.
 *
 * Estructura en disco:
 *   Offset 0     : ELF Header (52 bytes)
 *   Offset e_phoff: Program Header Table (e_phnum entradas de e_phentsize bytes)
 *
 * El cargador busca segmentos PT_LOAD y los copia a su p_vaddr en el espacio
 * del proceso. Los bytes p_memsz - p_filesz son BSS y se rellenan con ceros.
 */

/* Cabecera ELF32 (52 bytes) */
typedef struct {
    u8  e_ident[16];    /* magic + clase + endian + version + OS ABI */
    u16 e_type;         /* ET_EXEC = 2                                */
    u16 e_machine;      /* EM_386 = 3                                 */
    u32 e_version;
    u32 e_entry;        /* Punto de entrada virtual                   */
    u32 e_phoff;        /* Offset de la Program Header Table          */
    u32 e_shoff;        /* Offset de la Section Header Table (ignorar)*/
    u32 e_flags;
    u16 e_ehsize;       /* Tamanio de esta cabecera (52)              */
    u16 e_phentsize;    /* Tamanio de cada Program Header (32)        */
    u16 e_phnum;        /* Numero de Program Headers                  */
    u16 e_shentsize;
    u16 e_shnum;
    u16 e_shstrndx;
} __attribute__((packed)) elf32_hdr_t;

/* Program Header ELF32 (32 bytes) */
typedef struct {
    u32 p_type;     /* PT_LOAD = 1  */
    u32 p_offset;   /* Offset del segmento en el archivo             */
    u32 p_vaddr;    /* Direccion virtual destino                     */
    u32 p_paddr;    /* Direccion fisica (ignorar en sistemas virtuales)*/
    u32 p_filesz;   /* Bytes a copiar del archivo                    */
    u32 p_memsz;    /* Bytes totales en memoria (>= p_filesz)        */
    u32 p_flags;    /* PF_X=1, PF_W=2, PF_R=4                       */
    u32 p_align;    /* Alineacion (tipicamente PAGE_SIZE)             */
} __attribute__((packed)) elf32_phdr_t;

#define PT_LOAD     1           /* Segmento cargable           */
#define ELF_MAGIC   0x464C457F  /* "\x7FELF" little-endian u32 */

/*
 * elf_exec — carga el ELF llamado 'name' desde el directorio raiz ext2
 * y lo registra en el scheduler como una nueva tarea de usuario.
 */
void elf_exec(const char *name);

#endif
