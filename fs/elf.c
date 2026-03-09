#include "elf.h"
#include "ext2.h"
#include "pmm.h"
#include "paging.h"
#include "scheduler.h"
#include "screen.h"
#include "types.h"

/*
 * elf.c — Cargador de binarios ELF32 desde el filesystem ext2.
 *
 * Flujo de elf_exec(name):
 *   1. Leer el archivo ELF desde ext2 a un buffer estatico.
 *   2. Validar el magic ELF.
 *   3. Clonar el Page Directory del kernel → Page Directory del proceso.
 *      Esto garantiza que el espacio kernel queda mapeado en el proceso
 *      (necesario para manejar interrupciones y syscalls).
 *   4. Para cada segmento PT_LOAD:
 *      a. Calcular el rango de paginas virtuales a mapear.
 *      b. Alocar una pagina fisica por PMM (identity-mapped en el kernel).
 *      c. Zerar la pagina y copiar los bytes del archivo.
 *      d. Mapear la pagina en el PD del proceso con flags USER.
 *   5. Alocar pila de usuario (1 pagina en 0x40100000) e inicializarla a 0.
 *   6. Alocar pila kernel (1 pagina fisica) para las entradas a ring 0.
 *   7. Registrar la tarea en el scheduler con sched_add_task().
 */

#define MAX_ELF_SIZE  65536   /* 64 KB — suficiente para binarios simples */
#define USER_STACK_VIRT 0x40100000u

/* Paginacion: flags de entrada de page table */
#define PG_PRESENT  1u
#define PG_WRITE    2u
#define PG_USER     4u

/* Buffer estatico para el ELF (fuera de la pila, evita desbordamiento) */
static u8 elf_buf[MAX_ELF_SIZE];

/* ── proc_alloc_pd ─────────────────────────────────────────────────────────
 * Aloca una nueva pagina para el Page Directory del proceso y la inicializa
 * como copia del PD del kernel (KERNEL_PD_ADDR). El proceso hereda todo el
 * mapeo kernel pero tendra su propio espacio de usuario.
 */
static u32 proc_alloc_pd(void)
{
    u32 *kpd = (u32 *) KERNEL_PD_ADDR;
    u32  pd_phys = pmm_alloc_page();
    u32 *pd      = (u32 *) pd_phys;
    u32  i;

    for (i = 0; i < 1024; i++)
        pd[i] = kpd[i];

    return pd_phys;
}

/* ── proc_map_page ─────────────────────────────────────────────────────────
 * Mapea la pagina virtual 'virt' a la pagina fisica 'phys' en el PD
 * ubicado en 'pd_phys'. Si la Page Table del directorio no existe la crea.
 * Equivalente a vmm_map_page pero sobre un PD arbitrario (no el del kernel).
 */
static void proc_map_page(u32 pd_phys, u32 virt, u32 phys, u32 flags)
{
    u32  pd_idx = virt >> 22;
    u32  pt_idx = (virt >> 12) & 0x3FF;
    u32 *pd     = (u32 *) pd_phys;
    u32 *pt;
    u32  i;

    if (!(pd[pd_idx] & PG_PRESENT)) {
        u32 pt_phys = pmm_alloc_page();
        pt = (u32 *) pt_phys;
        for (i = 0; i < 1024; i++) pt[i] = 0;
        pd[pd_idx] = pt_phys | PG_PRESENT | PG_WRITE | PG_USER;
    }

    pt = (u32 *)(pd[pd_idx] & ~0xFFFu);
    pt[pt_idx] = phys | flags;
}

/* ── elf_exec ──────────────────────────────────────────────────────────────
 * Carga el ELF 'name' desde el directorio raiz ext2 y lo registra
 * en el scheduler como una nueva tarea de usuario ring-3.
 */
void elf_exec(const char *name)
{
    elf32_hdr_t  *hdr;
    u32           ino, size;
    u32           pd_phys, kstack_phys;
    u16           i;

    /* ── 1. Leer el archivo desde ext2 ── */
    ino = ext2_find(EXT2_ROOT_INODE, name);
    if (!ino) {
        print("  [!!] ELF: archivo no encontrado: ");
        print(name);
        print("\n");
        return;
    }

    size = ext2_read_file(ino, elf_buf, MAX_ELF_SIZE);
    if (size < 52) {
        print("  [!!] ELF: archivo demasiado pequeno\n");
        return;
    }

    /* ── 2. Validar magic ELF ── */
    hdr = (elf32_hdr_t *) elf_buf;
    if (hdr->e_ident[0] != 0x7F || hdr->e_ident[1] != 'E' ||
        hdr->e_ident[2] != 'L'  || hdr->e_ident[3] != 'F') {
        print("  [!!] ELF: magic invalido\n");
        return;
    }

    /* ── 3. Crear Page Directory del proceso ── */
    pd_phys = proc_alloc_pd();

    /* ── 4. Cargar segmentos PT_LOAD ── */
    for (i = 0; i < hdr->e_phnum; i++) {
        elf32_phdr_t *ph = (elf32_phdr_t *)
            (elf_buf + hdr->e_phoff + (u32)i * hdr->e_phentsize);
        u32 vbase, vend, vaddr;

        if (ph->p_type != PT_LOAD) continue;

        /* Rango de paginas virtuales a mapear (alineado a 4KB) */
        vbase = ph->p_vaddr & ~0xFFFu;
        vend  = (ph->p_vaddr + ph->p_memsz + 0xFFFu) & ~0xFFFu;

        for (vaddr = vbase; vaddr < vend; vaddr += 4096) {
            u32  frame = pmm_alloc_page();
            u8  *fp    = (u8 *) frame;
            u32  j;

            /* a. Zerar la pagina fisica (cubre el BSS) */
            for (j = 0; j < 4096; j++) fp[j] = 0;

            /* b. Copiar bytes del archivo que corresponden a esta pagina */
            if (vaddr < ph->p_vaddr + ph->p_filesz) {
                u32 src_off = ph->p_offset + (vaddr - ph->p_vaddr);
                u32 bytes   = 4096;
                u32 avail   = ph->p_offset + ph->p_filesz - src_off;
                if (bytes > avail) bytes = avail;
                for (j = 0; j < bytes; j++)
                    fp[j] = elf_buf[src_off + j];
            }

            /* c. Mapear en el PD del proceso con acceso usuario */
            proc_map_page(pd_phys, vaddr, frame,
                          PG_PRESENT | PG_WRITE | PG_USER);

        }
    }

    /* ── 5. Pila de usuario: 1 pagina en USER_STACK_VIRT ── */
    {
        u32  frame = pmm_alloc_page();
        u8  *fp    = (u8 *) frame;
        u32  j;
        for (j = 0; j < 4096; j++) fp[j] = 0;
        proc_map_page(pd_phys, USER_STACK_VIRT, frame,
                      PG_PRESENT | PG_WRITE | PG_USER);
    }

    /* ── 6. Pila kernel para la tarea (ring 0) ── */
    kstack_phys = pmm_alloc_page();

    /* ── 7. Registrar en el scheduler ── */
    /*
     * ESP usuario apunta al final de la pagina de pila menos 16 bytes
     * (convencion: la pila crece hacia abajo, reservamos algo de espacio).
     */
    sched_add_task(hdr->e_entry,         /* EIP: punto de entrada ELF    */
                   0x23,                  /* CS:  ring-3 code selector    */
                   0x202,                 /* EFLAGS: IF=1                 */
                   USER_STACK_VIRT + 0xFF0, /* ESP usuario               */
                   0x33,                  /* SS:  ring-3 stack selector   */
                   pd_phys,               /* CR3: PD del proceso          */
                   kstack_phys + 4096);   /* ESP0: tope pila kernel       */

    print("  [OK] ELF: '");
    print(name);
    print("' cargado y registrado en el scheduler\n");
}
