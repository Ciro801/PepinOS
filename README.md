# PepinOS

OS educativo x86-32 construido paso a paso siguiendo el tutorial
[Réaliser son propre système d'exploitation](https://michelizza.developpez.com/realiser-son-propre-systeme/)
de Florent Michelizza.

El objetivo es entender cómo funciona un sistema operativo real desde cero:
boot, modo protegido, interrupciones, paginación, multitarea, señales y
sistema de archivos — todo en C y ensamblador x86, sin librerías externas.

---

## Construcción y ejecución

### Requisitos

```
gcc (multilib / i686-elf)   nasm   ld   grub   mtools   qemu-system-i386
```

En Arch Linux:
```bash
sudo pacman -S gcc nasm binutils grub mtools qemu
```

### Comandos

| Comando       | Acción                                                  |
|---------------|---------------------------------------------------------|
| `make`        | Compila el kernel y los programas de usuario            |
| `make run`    | Compila, crea disco con GRUB y lanza QEMU               |
| `make debug`  | Igual que `run` pero con `-d int` (traza interrupciones)|
| `make clean`  | Borra todos los binarios y la imagen de disco           |

```bash
make run
```

El kernel arranca desde GRUB, monta la partición ext2 y lanza `shell.elf`
como primer proceso de usuario en ring 3.

---

## Shell interactivo

```
  PepinOS Shell  (paso 24 - señales POSIX)
  help: comandos disponibles
  Ctrl+C envía SIGINT | 'raise N' envía la señal N

pepinos$
```

| Comando          | Descripción                              |
|------------------|------------------------------------------|
| `help`           | Lista los comandos disponibles           |
| `ls`             | Muestra el directorio raíz de la partición |
| `cat <archivo>`  | Imprime el contenido de un archivo       |
| `echo <texto>`   | Repite el texto en pantalla              |
| `clear`          | Limpia la pantalla                       |
| `raise <N>`      | Envía la señal N al proceso actual       |

**Señales activas:** `Ctrl+C` → SIGINT · `raise 10` → SIGUSR1 · `raise 15` → SIGTERM

---

## Arquitectura del sistema

```
┌─────────────────────────────────────────────────────┐
│                  Espacio de usuario (ring 3)         │
│   shell.elf / hello.elf                             │
│   INT 0x30 → syscalls                               │
└──────────────────────┬──────────────────────────────┘
                       │ trap gate IDT[0x30]
┌──────────────────────▼──────────────────────────────┐
│                     Kernel (ring 0)                  │
│                                                      │
│  ┌──────────┐  ┌──────────┐  ┌────────────────────┐ │
│  │   arch/  │  │   mm/    │  │     process/       │ │
│  │  GDT IDT │  │ PMM  VMM │  │ scheduler syscall  │ │
│  │  TSS ASM │  │  paging  │  │  task  signal      │ │
│  └──────────┘  └──────────┘  └────────────────────┘ │
│                                                      │
│  ┌──────────┐  ┌──────────┐  ┌────────────────────┐ │
│  │ drivers/ │  │   fs/    │  │       lib/         │ │
│  │ screen   │  │  ext2    │  │  list  string      │ │
│  │ keyboard │  │  vfs     │  │  types             │ │
│  │  ide     │  │  elf     │  └────────────────────┘ │
│  └──────────┘  └──────────┘                         │
└─────────────────────────────────────────────────────┘
                       │
┌──────────────────────▼──────────────────────────────┐
│              Disco IDE — imagen ext2                 │
│  /boot/grub/grub.cfg   /boot/kernel.elf             │
│  shell.elf   hello.elf   hola.txt                   │
└─────────────────────────────────────────────────────┘
```

---

## Mapa de memoria físico

```
Dirección física     Contenido
─────────────────────────────────────────────────────
0x00000000-0x000FFFFF  Reservado (BIOS, IVT, etc.)
0x00100000-0x0011FFFF  Kernel ELF (128 KB)
0x00120000             Pila inicial del kernel (crece hacia abajo)
0x00200000             Page directory del kernel (KERNEL_PD_ADDR)
0x00201000-0x00204FFF  Page tables del kernel (identity map primeros 16 MB)
0x00205000-0x00208FFF  Page directories/tables de tareas A y B
0x00300000-0x00300FFF  Código de tarea A (antes de ELF loader)
0x00301000-0x00301FFF  Código de tarea B (antes de ELF loader)
0x0030E000-0x0030EFFF  Pila de usuario de tarea B
0x0030F000-0x0030FFFF  Pila de usuario de tarea A
0x00320000-0x00320FFF  Pila kernel de tarea A (ring 0, TSS.esp0)
0x00322000-0x00322FFF  Pila kernel de tarea B (ring 0, TSS.esp0)
0x00400000+            Bitmap PMM (Physical Memory Manager)

── Espacio de usuario (virtual, por proceso) ──────────────────────────
0x40000000             Base de código ELF (segmentos PT_LOAD)
0x40100000             Pila de usuario de procesos ELF
```

---

## Syscalls (INT 0x30)

| EAX | Nombre         | Argumentos                          | Retorno        |
|-----|----------------|-------------------------------------|----------------|
|  0  | SYS_PRINT      | EBX = `const char *s`               | —              |
|  1  | SYS_EXIT       | —                                   | —              |
|  2  | SYS_SETCOLOR   | EBX = `u8 attr` (VGA color byte)    | —              |
|  3  | SYS_OPEN       | EBX = `const char *path`            | EAX = fd o -1  |
|  4  | SYS_CLOSE      | EBX = `int fd`                      | —              |
|  5  | SYS_READ       | EBX = fd, ECX = buf, EDX = count    | EAX = bytes    |
|  6  | SYS_GETCHAR    | —                                   | EAX = char o 0 |
|  7  | SYS_LS         | —                                   | —              |
|  8  | SYS_SIGACTION  | EBX = signum, ECX = handler_addr    | —              |
|  9  | SYS_KILL       | EBX = signum                        | —              |
| 10  | SYS_SIGRETURN  | —                                   | —              |

Los programas de usuario invocan syscalls así:
```c
asm volatile("int $0x30" : "=a"(ret) : "a"(SYS_READ), "b"(fd), "c"(buf), "d"(count));
```

---

## Señales POSIX

| Señal   | Número | Generada por                     |
|---------|--------|----------------------------------|
| SIGINT  |   2    | Ctrl+C en el teclado             |
| SIGUSR1 |  10    | `raise 10` en el shell           |
| SIGUSR2 |  12    | `sys_kill(12)` desde código      |
| SIGTERM |  15    | `raise 15` en el shell           |

**Ciclo de entrega:**
1. `sched_signal(sig)` marca el bit en `task_t.sig_pending`
2. En el siguiente tick del timer, `do_switch()` detecta la señal pendiente
3. Inyecta un call frame en la pila de usuario (signum + ret addr dummy)
4. Redirige EIP al handler registrado con `sys_sigaction()`
5. El handler ejecuta y llama `sys_sigreturn()` para restaurar el contexto original

```c
// Ejemplo: registrar e implementar un handler
static void mi_handler(int sig) {
    sys_print("señal recibida\n");
    sys_sigreturn();          // obligatorio — no usar 'return'
}

sys_sigaction(SIGINT, mi_handler);
```

---

## Estructura del proyecto

```
pepinos_os/
├── arch/           GDT, IDT, TSS, ensamblador de excepciones/IRQs, linker.ld
├── boot/           Cabecera Multiboot, código de arranque
├── drivers/        screen (VGA), keyboard (8042 + ring buffer), ide (PIO LBA28)
├── fs/             ext2 (lectura), vfs (pool de nodos + fd), elf (loader ELF32)
├── kernel/         kmain() — punto de entrada en C
├── lib/            types.h, list.h (listas intrusivas), string.h/c
├── mm/             pmm (bitmap), vmm (map/unmap + invlpg), paging (init)
├── process/        scheduler (round-robin + señales), syscall, task, signal
├── user/           shell.c, hello.c, shell.h, link_user.ld
├── Makefile        Build principal + generación del disco GRUB
└── README.md
```

---

## Detalles técnicos de compilación

```makefile
CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall \
         -mno-sse -mno-mmx
```

**Por qué `-mno-sse -mno-mmx`:** GCC puede emitir instrucciones SSE2 (`movdqa`,
`movups`) para inicializar arrays locales. El kernel no habilita `CR4.OSFXSR`,
por lo que ejecutar SSE dispara una excepción `#UD` (Invalid Opcode, vector 6).
Desactivar SSE/MMX en el compilador evita este problema.

**Disco de arranque:**
```
Sector 0          : MBR con GRUB boot.img (446 bytes)
Sectores 1-2047   : GRUB core.img (gap pre-partición)
Sector 2048+      : Partición ext2 (32 MB total)
  └─ /boot/grub/grub.cfg
  └─ /boot/kernel.elf
  └─ shell.elf
  └─ hello.elf
  └─ hola.txt
```

---

## Pasos del tutorial — estado de implementación

### FASE 1 — Arranque

| Paso | Capítulo | Qué construye                                          | Estado |
|------|----------|--------------------------------------------------------|--------|
|  1   | II       | Sector de boot que muestra "Hello World"               | ✅     |
|  2   | III      | Bootloader que carga el kernel desde disco             | ✅     |
|  3   | IV       | Bootloader con Modo Protegido (32-bit) + GDT           | ✅     |

### FASE 2 — Kernel básico en C

| Paso | Capítulo | Qué construye                                          | Estado |
|------|----------|--------------------------------------------------------|--------|
|  4   | V        | Kernel mixto ASM+C, luego kernel 100% en C             | ✅     |
|  5   | VI       | Kernel que recarga su propia GDT                       | ✅     |
|  6   | VII      | Teoría del PIC 8259A + IDT                             | ✅     |
|  7   | VIII     | Interrupciones completas (excepciones + IRQs)          | ✅     |
|  8   | IX       | Driver de teclado (8042 + cursor hardware)             | ✅     |

### FASE 3 — Procesos y memoria

| Paso | Capítulo | Qué construye                                          | Estado |
|------|----------|--------------------------------------------------------|--------|
|  9   | X        | Primera tarea de usuario, TSS, cambio de contexto      | ✅     |
| 10   | XI       | System calls (INT 0x30, trap gate DPL=3)               | ✅     |
| 11   | XII      | Paginación básica (identity mapping, Page Fault)       | ✅     |
| 12   | XIII     | Paginación por tarea, espacios de memoria aislados     | ✅     |

### FASE 4 — Multitarea y sistema de archivos

| Paso | Capítulo | Qué construye                                          | Estado |
|------|----------|--------------------------------------------------------|--------|
| 13   | XIV      | Scheduler round-robin por IRQ0 (timer)                 | ✅     |
| 14   | XV       | Scheduler con syscalls preemptibles, pila kernel/tarea | ✅     |
| 15   | XVI      | Boot con GRUB (estándar Multiboot)                     | ✅     |
| 16   | XVII     | PMM (bitmap) + VMM (map/unmap + invlpg)                | ✅     |
| 17   | XVIII    | Driver IDE PIO (LBA28, puertos 0x1F0-0x1F7)            | ✅     |
| 18   | XIX      | Sistema de archivos Ext2 (superblock, inodos, directorios) | ✅  |

### FASE 5 — Sistema completo

| Paso | Capítulo | Qué construye                                          | Estado |
|------|----------|--------------------------------------------------------|--------|
| 19   | XX       | ELF loader — carga y ejecuta binarios desde ext2       | ✅     |
| 20   | XXI      | Boot real desde disco IDE particionado con GRUB        | ✅     |
| 21   | XXII     | VFS + descriptores de archivo en `task_t`              | ✅     |
| 22   | XXIII    | Listas enlazadas intrusivas (estilo Linux/FreeBSD)     | ✅     |
| 23   | XXIV     | Shell interactivo (readline, comandos, colores VGA)    | ✅     |
| 24   | XXV      | Señales POSIX (sigaction, kill, sigreturn, Ctrl+C)     | ✅     |

---

## Anexos del tutorial (referencia)

- **A** — Compilación separada ASM+C en Unix
- **B** — Aritmética en base 16
- **C** — Bochs en modo debug
- **D** — Stack frames y argumentos en la pila
- **E** — Depurar el kernel con GDB
- **F** — Boot con GRUB2
