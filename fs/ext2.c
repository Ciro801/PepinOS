#include "ext2.h"
#include "ide.h"
#include "screen.h"
#include "types.h"

/*
 * Estado interno del driver.
 * Todos los buffers son globales (no en la pila) para evitar desbordamiento.
 */
static u32         block_size;          /* bytes por bloque (tipicamente 1024) */
static u32         inode_size;          /* bytes por inodo  (tipicamente 128)  */
static u32         inodes_per_group;
static ext2_bgd_t  bgd;                 /* primer Block Group Descriptor */
static u32         part_lba = 0;        /* LBA de inicio de la partición ext2  */

static u8 blk_a[4096];  /* buffer de bloques — uso general  */
static u8 blk_b[4096];  /* buffer de bloques — directorio   */

/* ── Lectura de bloques ─────────────────────────────────────────────────── */

/* ext2_set_partition — registra el LBA de inicio de la partición ext2.
 * Llamar antes de ext2_init() cuando el filesystem no empieza en el sector 0
 * (p.ej. cuando hay una tabla de particiones MBR con GRUB). */
void ext2_set_partition(u32 lba_start)
{
    part_lba = lba_start;
}

static void ext2_read_block(u32 block, u8 *buf)
{
    u32 sects = block_size / 512;
    u32 lba   = part_lba + block * sects;
    u32 i;
    for (i = 0; i < sects; i++)
        ide_read_sector(lba + i, buf + i * 512);
}

/* ── Inicializacion ─────────────────────────────────────────────────────── */

/*
 * ext2_init — Lee el superblock (sector 2, byte 1024 del disco),
 * valida el numero magico 0xEF53 y carga el primer Block Group Descriptor.
 *
 * Campos del superblock accedidos por offset (evita problemas de padding):
 *   +20  s_first_data_block  (u32)
 *   +24  s_log_block_size    (u32)  → block_size = 1024 << valor
 *   +40  s_inodes_per_group  (u32)
 *   +56  s_magic             (u16)
 *   +76  s_rev_level         (u32)
 *   +88  s_inode_size        (u16)  solo valido si rev_level >= 1
 */
void ext2_init(void)
{
    u8 sb[512];         /* un sector es suficiente para los campos que leemos */
    u32 log_bsz, rev, first_data;
    u16 magic, sb_inode_sz;
    u32 bgdt_block;
    u32 i;
    u8 *bgd_raw;

    ide_read_sector(part_lba + 2, sb); /* superblock: byte 1024 del FS = sector 2 relativo */

    magic      = *(u16*)(sb + 56);
    log_bsz    = *(u32*)(sb + 24);
    rev        = *(u32*)(sb + 76);
    first_data = *(u32*)(sb + 20);
    sb_inode_sz= *(u16*)(sb + 88);
    inodes_per_group = *(u32*)(sb + 40);

    if (magic != EXT2_MAGIC) {
        print("  [!!] Ext2: magic invalido — no es ext2\n");
        return;
    }

    block_size = 1024u << log_bsz;
    inode_size = (rev >= 1 && sb_inode_sz >= 128) ? sb_inode_sz : 128;

    /* El BGDT esta en el bloque siguiente al superblock */
    bgdt_block = first_data + 1;
    ext2_read_block(bgdt_block, blk_a);

    bgd_raw = (u8 *)&bgd;
    for (i = 0; i < sizeof(ext2_bgd_t); i++)
        bgd_raw[i] = blk_a[i];

    print("  [OK] Ext2: filesystem montado\n");
}

/* ── Lectura de inodos ──────────────────────────────────────────────────── */

static void ext2_read_inode(u32 inum, ext2_inode_t *inode)
{
    u32 idx    = inum - 1;
    u32 ipb    = block_size / inode_size;        /* inodos por bloque */
    u32 block  = bgd.bg_inode_table + idx / ipb;
    u32 offset = (idx % ipb) * inode_size;
    u32 i;
    u8 *raw;

    ext2_read_block(block, blk_a);

    raw = (u8 *)inode;
    for (i = 0; i < sizeof(ext2_inode_t); i++)
        raw[i] = blk_a[offset + i];
}

/* ── Directorio: listar ─────────────────────────────────────────────────── */

void ext2_ls(u32 dir_ino)
{
    ext2_inode_t inode;
    u32 off;

    ext2_read_inode(dir_ino, &inode);
    ext2_read_block(inode.i_block[0], blk_b);

    off = 0;
    while (off < inode.i_size && off < block_size) {
        ext2_dirent_t *de = (ext2_dirent_t *)(blk_b + off);
        if (de->rec_len == 0) break;
        if (de->inode != 0) {
            char name[256];
            u8 j;
            u8 *name_ptr = (u8 *)de + sizeof(ext2_dirent_t);
            for (j = 0; j < de->name_len; j++)
                name[j] = (char)name_ptr[j];
            name[de->name_len] = '\0';
            print("    ");
            print(name);
            print("\n");
        }
        off += de->rec_len;
    }
}

/* ── Directorio: buscar ─────────────────────────────────────────────────── */

u32 ext2_find(u32 dir_ino, const char *name)
{
    ext2_inode_t inode;
    u32 off;

    ext2_read_inode(dir_ino, &inode);
    ext2_read_block(inode.i_block[0], blk_b);

    off = 0;
    while (off < inode.i_size && off < block_size) {
        ext2_dirent_t *de = (ext2_dirent_t *)(blk_b + off);
        if (de->rec_len == 0) break;
        if (de->inode != 0) {
            u8 *name_ptr = (u8 *)de + sizeof(ext2_dirent_t);
            const char *p = name;
            u8 j;
            int match = 1;
            for (j = 0; j < de->name_len; j++) {
                if (*p == '\0' || *p != (char)name_ptr[j]) {
                    match = 0;
                    break;
                }
                p++;
            }
            if (match && *p == '\0')
                return de->inode;
        }
        off += de->rec_len;
    }
    return 0;
}

/* ── Lectura de archivos ────────────────────────────────────────────────── */

/*
 * ext2_inode_size — devuelve el tamaño en bytes del inodo 'inum'.
 * Usado por el VFS para rellenar fs_node_t.length sin leer el archivo entero.
 */
u32 ext2_inode_size(u32 inum)
{
    ext2_inode_t inode;
    ext2_read_inode(inum, &inode);
    return inode.i_size;
}

/*
 * ext2_pread — lectura posicionada: lee 'size' bytes desde el byte 'offset'
 * del archivo con inodo 'inum'. Maneja bloques sparse.
 * Usado por el VFS (vfs_read) para implementar sys_read con offset corriente.
 */
u32 ext2_pread(u32 inum, u8 *buf, u32 offset, u32 size)
{
    ext2_inode_t inode;
    u32 file_size, blk, copied, j, blk_off;

    ext2_read_inode(inum, &inode);
    file_size = inode.i_size;

    if (offset >= file_size) return 0;
    if (offset + size > file_size) size = file_size - offset;

    copied  = 0;
    blk     = offset / block_size;
    blk_off = offset % block_size;   /* byte de inicio dentro del primer bloque */

    while (copied < size && blk < 12) {
        if (inode.i_block[blk] == 0) {
            /* bloque sparse: rellenar con ceros */
            for (j = blk_off; j < block_size && copied < size; j++)
                buf[copied++] = 0;
        } else {
            ext2_read_block(inode.i_block[blk], blk_a);
            for (j = blk_off; j < block_size && copied < size; j++)
                buf[copied++] = blk_a[j];
        }
        blk++;
        blk_off = 0;   /* los bloques siguientes empiezan desde el byte 0 */
    }

    return copied;
}

u32 ext2_read_file(u32 inum, u8 *buf, u32 max_size)
{
    ext2_inode_t inode;
    u32 size, copied, blk, bytes, j;

    ext2_read_inode(inum, &inode);

    size   = (inode.i_size < max_size) ? inode.i_size : max_size;
    copied = 0;

    for (blk = 0; blk < 12 && copied < size; blk++) {
        bytes = block_size;
        if (copied + bytes > size)
            bytes = size - copied;
        if (inode.i_block[blk] == 0) {
            /* bloque sparse: el archivo tiene un hueco, rellenar con ceros */
            for (j = 0; j < bytes; j++)
                buf[copied + j] = 0;
        } else {
            ext2_read_block(inode.i_block[blk], blk_a);
            for (j = 0; j < bytes; j++)
                buf[copied + j] = blk_a[j];
        }
        copied += bytes;
    }

    return copied;
}
