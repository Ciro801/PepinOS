#include "types.h"
#include "task.h"
#include "tss.h"
#include "paging.h"
#include "screen.h"

void launch_task(void)
{
    /* Paso 1: copiar código de user_task a dirección física */
    u8 *src = (u8 *) user_task;
    u8 *dst = (u8 *) TASK_CODE_PHYS;
    u32 i;
    for (i = 0; i < 512; i++)
        dst[i] = src[i];

    kattr = 0x07;
    print("  [OK] Codigo copiado a 0x30000 (fisico)\n");

    /* Paso 2: crear espacio de memoria separado para la tarea */
    create_task_space(TASK_CODE_PHYS,  TASK_STACK_PHYS,
                      TASK_CODE_VIRT,  TASK_STACK_VIRT);
    print("  [OK] Espacio usuario creado\n");
    print("       virtual 0x0000 -> fisico 0x30000 (codigo)\n");
    print("       virtual 0x1000 -> fisico 0x3F000 (pila)\n");
    print("       virtual 0xC000 -> kernel mapeado\n\n");

    /* Paso 3: inicializar TSS */
    init_tss(0x20000);

    /* Paso 4: cargar TR */
    asm("movw $0x38, %ax \n"
        "ltr  %ax        \n");

    print("  [OK] TSS listo\n");
    print("  [OK] Cambiando a espacio usuario...\n\n");

    /* Paso 5: cambiar al Page Directory de la tarea */
    switch_to_task_space();

    /* Paso 6: iret a ring 3
     * La tarea ve su código en TASK_CODE_VIRT = 0x40000000
     * Su pila en TASK_STACK_VIRT + 0xFF0 = 0x40001FF0
     */
    asm("  mov $0x33, %ax        \n"  /* SS usuario  */
        "  push %eax             \n"
        "  push $0x40001FF0      \n"  /* ESP usuario */
        "  pushf                 \n"
        "  pop  %eax             \n"
        "  or   $0x200, %eax     \n"  /* IF=1 */
        "  push %eax             \n"
        "  mov  $0x23, %eax      \n"  /* CS usuario  */
        "  push %eax             \n"
        "  push $0x40000000      \n"  /* EIP = inicio del código virtual */
        "  iret                  \n");
}