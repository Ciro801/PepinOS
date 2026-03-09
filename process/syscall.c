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
 * frame: puntero al bloque de registros en la pila kernel.
 *   frame[0..3] = eax,ebx,ecx,edx del usuario (registros guardados por asm)
 *   frame[4..8] = EIP,CS,EFLAGS,ESP_user,SS  (iret frame empujado por CPU)
 *
 * eax = número de syscall
 * ebx = arg1, ecx = arg2, edx = arg3
 */
int syscall_handler(u32 *frame, u32 eax, u32 ebx, u32 ecx, u32 edx)
{
    switch (eax) {

    case SYS_PRINT:
        kattr = 0x0F;
        print((char *) ebx);
        return 0;

    case SYS_EXIT:
        kattr = 0x0C;
        print("\n[kernel] tarea terminada.\n");
        while (1);
        return 0;

    case SYS_SETCOLOR:
        kattr = (char) ebx;
        return 0;

    case SYS_OPEN: {
        task_t    *t    = sched_current_task();
        fs_node_t *node = vfs_open((const char *) ebx);
        int        fd;

        if (!node) return -1;
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
        task_t *t  = sched_current_task();
        int     fd = (int) ebx;

        if (fd < 3 || fd >= MAX_FD || !t->files[fd].used) return -1;
        vfs_close((fs_node_t *) t->files[fd].node);
        t->files[fd].node   = 0;
        t->files[fd].offset = 0;
        t->files[fd].used   = 0;
        return 0;
    }

    case SYS_READ: {
        task_t    *t     = sched_current_task();
        int        fd    = (int) ebx;
        u8        *buf   = (u8 *) ecx;
        u32        count = edx;
        fs_node_t *node;
        u32        n;

        if (fd < 3 || fd >= MAX_FD || !t->files[fd].used) return -1;
        node = (fs_node_t *) t->files[fd].node;
        n    = vfs_read(node, t->files[fd].offset, count, buf);
        t->files[fd].offset += n;
        return (int) n;
    }

    case SYS_GETCHAR:
        return (int)(unsigned char) kb_getchar();

    case SYS_LS:
        kattr = 0x0E;
        ext2_ls(EXT2_ROOT_INODE);
        return 0;

    case SYS_SIGACTION: {
        /* ebx=signum, ecx=handler_addr (0 para desinstalar) */
        task_t *t   = sched_current_task();
        int     sig = (int) ebx;

        if (!t || sig <= 0 || sig >= NSIG) return -1;
        t->sig_handlers[sig] = ecx;
        return 0;
    }

    case SYS_KILL: {
        /* ebx=signum — señal al proceso actual (self-kill simplificado) */
        task_t *t   = sched_current_task();
        int     sig = (int) ebx;

        if (!t || sig <= 0 || sig >= NSIG) return -1;
        t->sig_pending |= (1u << sig);
        return 0;
    }

    case SYS_SIGRETURN: {
        /*
         * Restaura el contexto guardado en sig_saved_regs antes de la señal.
         *
         * Modificamos el iret frame (frame[4..8]) directamente en la pila
         * kernel para que el iret siguiente salte al EIP original.
         *
         * sig_saved_regs layout (igual que ctx en do_switch):
         *   [0]=EDI [1]=ESI [2]=EBP [3]=ESP_ [4]=EBX [5]=EDX [6]=ECX [7]=EAX
         *   [8]=EIP [9]=CS [10]=EFLAGS [11]=ESP_user [12]=SS
         *
         * frame layout (syscall, ver _idt_syscall en idt_asm.asm):
         *   [0]=eax [1]=ebx [2]=ecx [3]=edx
         *   [4]=EIP [5]=CS [6]=EFLAGS [7]=ESP_user [8]=SS
         */
        task_t *t = sched_current_task();

        if (!t || !t->sig_in_handler) return -1;

        /* Restaurar registros de propósito general */
        frame[0] = t->sig_saved_regs[7];   /* EAX */
        frame[1] = t->sig_saved_regs[4];   /* EBX */
        frame[2] = t->sig_saved_regs[6];   /* ECX */
        frame[3] = t->sig_saved_regs[5];   /* EDX */

        /* Restaurar iret frame — la CPU los usará al ejecutar iret */
        frame[4] = t->sig_saved_regs[8];   /* EIP      */
        frame[5] = t->sig_saved_regs[9];   /* CS       */
        frame[6] = t->sig_saved_regs[10];  /* EFLAGS   */
        frame[7] = t->sig_saved_regs[11];  /* ESP_user */
        frame[8] = t->sig_saved_regs[12];  /* SS       */

        t->sig_in_handler = 0;
        return 0;
    }

    default:
        return -1;
    }
}
