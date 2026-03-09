#ifndef _EXT2_H_
#define _EXT2_H_

#include "types.h"

/*
 * Ext2 — Second Extended Filesystem (lectura en modo PIO via IDE)
 *
 * Estructura en disco (bloques de 1KB):
 *   Bloque 0 : boot block (ignorado)
 *   Bloque 1 : Superblock  (magic = 0xEF53)
 *   Bloque 2 : Block Group Descriptor Table (BGDT)
 *   bgd.bg_inode_table : tabla de inodos (128 bytes/inodo)
 *   Inodo 2  : directorio raiz
 *
 * Solo se implementa lectura de bloques directos (hasta 12x1KB = 12KB/archivo).
 */

#define EXT2_MAGIC        0xEF53
#define EXT2_ROOT_INODE   2

/* Block Group Descriptor (32 bytes) */
typedef struct {
    u32 bg_block_bitmap;
    u32 bg_inode_bitmap;
    u32 bg_inode_table;         /* primer bloque de la tabla de inodos */
    u16 bg_free_blocks_count;
    u16 bg_free_inodes_count;
    u16 bg_used_dirs_count;
    u16 bg_pad;
    u8  bg_reserved[12];
} __attribute__((packed)) ext2_bgd_t;

/* Inodo (128 bytes en revision 0) */
typedef struct {
    u16 i_mode;
    u16 i_uid;
    u32 i_size;         /* tamanio en bytes */
    u32 i_atime;
    u32 i_ctime;
    u32 i_mtime;
    u32 i_dtime;
    u16 i_gid;
    u16 i_links_count;
    u32 i_blocks;       /* bloques de 512 bytes usados */
    u32 i_flags;
    u32 i_osd1;
    u32 i_block[15];    /* [0-11] directos, [12] indirecto simple */
    u32 i_generation;
    u32 i_file_acl;
    u32 i_dir_acl;
    u32 i_faddr;
    u8  i_osd2[12];
} __attribute__((packed)) ext2_inode_t;

/* Cabecera de entrada de directorio */
typedef struct {
    u32 inode;          /* numero de inodo (1-based); 0 = entrada libre */
    u16 rec_len;        /* tamanio total (cabecera + nombre + padding)   */
    u8  name_len;       /* longitud del nombre (SIN terminador null)     */
    u8  file_type;      /* 1=regular, 2=directorio                       */
    /* name[name_len] sigue inmediatamente — NO null-terminated          */
} __attribute__((packed)) ext2_dirent_t;

void ext2_set_partition(u32 lba_start);
void ext2_init(void);
void ext2_ls(u32 dir_ino);
u32  ext2_find(u32 dir_ino, const char *name);
u32  ext2_inode_size(u32 inum);
u32  ext2_pread(u32 inum, u8 *buf, u32 offset, u32 size);
u32  ext2_read_file(u32 inum, u8 *buf, u32 max_size);

#endif
