#ifndef _VFS_H_
#define _VFS_H_

#include "types.h"

/*
 * VFS — Virtual File System
 *
 * Capa de abstracción entre el código del proceso (sys_open/read/close)
 * y el driver de filesystem concreto (ext2, FAT, ramfs…).
 *
 * Jerarquía:
 *   proceso → fd_t (offset actual) → fs_node_t (nodo abstracto)
 *           → node->read() → ext2_pread (implementación real)
 */

/* Tipos de nodo */
#define FS_FILE  0x01
#define FS_DIR   0x02

/* Tamaño del pool de nodos VFS simultáneos */
#define VFS_POOL_SIZE  8

/* Número máximo de archivos abiertos por proceso */
#define MAX_FD  8

typedef struct fs_node fs_node_t;

/* Función de lectura: lee 'size' bytes desde 'offset', retorna bytes leídos */
typedef u32 (*vfs_read_fn)(fs_node_t *node, u32 offset, u32 size, u8 *buf);

/*
 * fs_node_t — nodo VFS abstracto.
 * Representa un archivo o directorio independientemente del filesystem.
 */
struct fs_node {
    char        name[64];   /* nombre del archivo/directorio */
    u32         flags;      /* FS_FILE o FS_DIR              */
    u32         inode;      /* número de inodo (específico del FS) */
    u32         length;     /* tamaño del archivo en bytes   */
    vfs_read_fn read;       /* operación de lectura          */
    int         used;       /* 1 = nodo activo en el pool    */
};

/*
 * fd_t — descriptor de archivo abierto.
 * Cada proceso mantiene un array de estos (task_t.files[MAX_FD]).
 * Se usa void* para el nodo y evitar dependencia circular de headers.
 */
typedef struct {
    void *node;    /* fs_node_t * */
    u32   offset;  /* posición actual de lectura/escritura */
    int   used;    /* 1 = slot ocupado */
} fd_t;

/* ── API pública ──────────────────────────────────────────────────────────── */

void       vfs_init(void);
fs_node_t *vfs_open(const char *path);
void       vfs_close(fs_node_t *node);
u32        vfs_read(fs_node_t *node, u32 offset, u32 size, u8 *buf);

#endif
