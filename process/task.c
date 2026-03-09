#include "types.h"
#include "task.h"
#include "tss.h"
#include "paging.h"
#include "scheduler.h"
#include "screen.h"

void launch_tasks(void)
{
    u8 *src, *dst;
    u32 i;

    kattr = 0x07;

    /* ── Tarea A ── */
    src = (u8 *) user_task;
    dst = (u8 *) TASK_A_CODE_PHYS;
    for (i = 0; i < 1024; i++)
        dst[i] = src[i];
    print("  [OK] Tarea A copiada a 0x30000\n");

    create_task_space(TASK_A_CODE_PHYS, TASK_A_STACK_PHYS,
                      TASK_CODE_VIRT,   TASK_STACK_VIRT,
                      TASK_A_PD_ADDR,   TASK_A_PT_ADDR);
    print("  [OK] Espacio de memoria A creado\n");

    /* ── Tarea B ── */
    src = (u8 *) user_task_b;
    dst = (u8 *) TASK_B_CODE_PHYS;
    for (i = 0; i < 1024; i++)
        dst[i] = src[i];
    print("  [OK] Tarea B copiada a 0x31000\n");

    create_task_space(TASK_B_CODE_PHYS, TASK_B_STACK_PHYS,
                      TASK_CODE_VIRT,   TASK_STACK_VIRT,
                      TASK_B_PD_ADDR,   TASK_B_PT_ADDR);
    print("  [OK] Espacio de memoria B creado\n");

    /* ── TSS: pila kernel de tarea A (la primera en ejecutarse) ── */
    init_tss(TASK_A_KSTACK_TOP);
    asm("movw $0x38, %ax \n"
        "ltr  %ax        \n");
    print("  [OK] TSS listo (pilas kernel separadas)\n");

    /* ── Registrar tareas en el scheduler ── */
    sched_add_task(TASK_CODE_VIRT, 0x23, 0x202,
                   TASK_STACK_VIRT + 0xFF0, 0x33, TASK_A_PD_ADDR, TASK_A_KSTACK_TOP);
    sched_add_task(TASK_CODE_VIRT, 0x23, 0x202,
                   TASK_STACK_VIRT + 0xFF0, 0x33, TASK_B_PD_ADDR, TASK_B_KSTACK_TOP);
    print("  [OK] Scheduler: 2 tareas registradas\n");

    print("  [OK] Tareas A y B registradas. Llamar sched_enter() para iniciar.\n");
}

/*
 * sched_enter — Activa la multitarea saltando a ring 3 con iret.
 *
 * Debe llamarse DESPUES de registrar todas las tareas (launch_tasks + elf_exec).
 * Carga el CR3 de la tarea A y hace iret hacia ring 3. A partir de aqui
 * el scheduler IRQ0 conmutara entre todas las tareas registradas.
 * Esta funcion nunca retorna.
 */
void sched_enter(void)
{
    asm volatile("mov %0, %%cr3" : : "r"((u32)TASK_A_PD_ADDR));

    asm("  mov $0x33, %ax         \n"   /* SS usuario  */
        "  push %eax              \n"
        "  push $0x40001FF0       \n"   /* ESP usuario */
        "  pushf                  \n"
        "  pop  %eax              \n"
        "  or   $0x200, %eax      \n"   /* IF=1        */
        "  push %eax              \n"
        "  mov  $0x23, %eax       \n"   /* CS usuario  */
        "  push %eax              \n"
        "  push $0x40000000       \n"   /* EIP = inicio tarea A */
        "  iret                   \n");
}
