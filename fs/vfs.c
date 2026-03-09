#include "vfs.h"
#include "ext2.h"

/*
 * vfs.c — Virtual File System (capa de abstracción sobre ext2)
 *
 * Implementa un pool estático de fs_node_t. No hay malloc — cada nodo se
 * reserva del pool al hacer vfs_open() y se libera con vfs_close().
 *
 * Por ahora solo soporta el directorio raíz del ext2 montado.
 * Rutas: "hola.txt" o "/hola.txt" (la '/' inicial se ignora).
 */

static fs_node_t node_pool[VFS_POOL_SIZE];

/* ── Backend ext2 ─────────────────────────────────────────────────────────── */

static u32 ext2_node_read(fs_node_t *node, u32 offset, u32 size, u8 *buf)
{
    return ext2_pread(node->inode, buf, offset, size);
}

/* ── API pública ──────────────────────────────────────────────────────────── */

void vfs_init(void)
{
    int i;
    for (i = 0; i < VFS_POOL_SIZE; i++)
        node_pool[i].used = 0;
}

/*
 * vfs_open — abre un archivo del directorio raíz ext2.
 * path: nombre de archivo (con o sin '/' inicial).
 * Retorna un fs_node_t * del pool, o NULL si no se encuentra / pool lleno.
 */
fs_node_t *vfs_open(const char *path)
{
    fs_node_t *node = 0;
    u32  ino;
    int  i;

    /* Buscar slot libre en el pool */
    for (i = 0; i < VFS_POOL_SIZE; i++) {
        if (!node_pool[i].used) {
            node = &node_pool[i];
            break;
        }
    }
    if (!node) return 0;   /* pool lleno */

    /* Ignorar '/' inicial */
    if (*path == '/') path++;

    /* Buscar el inodo en el directorio raíz ext2 */
    ino = ext2_find(EXT2_ROOT_INODE, path);
    if (!ino) return 0;

    /* Rellenar el nodo */
    node->inode  = ino;
    node->flags  = FS_FILE;
    node->read   = ext2_node_read;
    node->length = ext2_inode_size(ino);
    node->used   = 1;

    /* Copiar nombre (sin el '/' ya eliminado) */
    for (i = 0; i < 63 && path[i]; i++)
        node->name[i] = path[i];
    node->name[i] = '\0';

    return node;
}

void vfs_close(fs_node_t *node)
{
    if (node) node->used = 0;
}

u32 vfs_read(fs_node_t *node, u32 offset, u32 size, u8 *buf)
{
    if (!node || !node->read) return 0;
    return node->read(node, offset, size, buf);
}
