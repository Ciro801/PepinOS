#include "types.h"
#include "screen.h"
#include "syscall.h"
#include "scheduler.h"
#include "vfs.h"
#include "keyboard.h"
#include "ext2.h"

/*
 * syscall_handler — despachador de syscalls
 * Llamado desde _idt_syscall en idt_asm.asm.
 * Retorna int: el valor se deja en EAX y llega al proceso usuario.
 *
 * eax = número de syscall
 * ebx = arg1, ecx = arg2, edx = arg3
 */
int syscall_handler(u32 eax, u32 ebx, u32 ecx, u32 edx)
{
    switch (eax) {

    case SYS_PRINT:
        /* ebx = puntero a string (espacio usuario) */
        kattr = 0x0F;
        print((char *) ebx);
        return 0;

    case SYS_EXIT:
        kattr = 0x0C;
        print("\n[kernel] tarea terminada.\n");
        while (1);
        return 0;

    case SYS_SETCOLOR:
        /* ebx = nuevo atributo de color VGA */
        kattr = (char) ebx;
        return 0;

    case SYS_OPEN: {
        /* ebx = puntero a path (espacio usuario) */
        task_t    *t    = sched_current_task();
        fs_node_t *node = vfs_open((const char *) ebx);
        int        fd;

        if (!node) return -1;

        /* Slots 0,1,2 reservados para stdin/stdout/stderr */
        for (fd = 3; fd < MAX_FD; fd++) {
            if (!t->files[fd].used) {
                t->files[fd].node   = node;
                t->files[fd].offset = 0;
                t->files[fd].used   = 1;
                return fd;
            }
        }
        vfs_close(node);
        return -1;
    }

    case SYS_CLOSE: {
        /* ebx = fd */
        task_t *t  = sched_current_task();
        int     fd = (int) ebx;

        if (fd < 3 || fd >= MAX_FD || !t->files[fd].used)
            return -1;

        vfs_close((fs_node_t *) t->files[fd].node);
        t->files[fd].node   = 0;
        t->files[fd].offset = 0;
        t->files[fd].used   = 0;
        return 0;
    }

    case SYS_READ: {
        /* ebx=fd, ecx=buf (puntero usuario), edx=count */
        task_t    *t     = sched_current_task();
        int        fd    = (int) ebx;
        u8        *buf   = (u8 *) ecx;
        u32        count = edx;
        fs_node_t *node;
        u32        n;

        if (fd < 3 || fd >= MAX_FD || !t->files[fd].used)
            return -1;

        node = (fs_node_t *) t->files[fd].node;
        n    = vfs_read(node, t->files[fd].offset, count, buf);
        t->files[fd].offset += n;
        return (int) n;
    }

    case SYS_GETCHAR:
        /* Devuelve el siguiente carácter del ring buffer, o 0 si vacío */
        return (int)(unsigned char) kb_getchar();

    case SYS_LS:
        /* Lista el directorio raíz en pantalla */
        kattr = 0x0E;
        ext2_ls(EXT2_ROOT_INODE);
        return 0;

    default:
        return -1;
    }
}
