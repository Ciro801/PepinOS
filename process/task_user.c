/*
 * task_user.c — tareas de usuario de placeholder (ring 3)
 *
 * Para la Fase 5 estas tareas serán reemplazadas por procesos ELF cargados
 * desde disco. Por ahora son bucles idle que permiten verificar que el
 * scheduler conmuta contexto correctamente sin ensuciar la pantalla.
 */

void user_task(void)
{
    volatile int i;
    while (1)
        for (i = 0; i < 5000000; i++);
}

void user_task_b(void)
{
    volatile int i;
    while (1)
        for (i = 0; i < 5000000; i++);
}
