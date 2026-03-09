# PepinOS

## 🗺️ Plan completo — Pépin OS

Basado exactamente en el índice del tutorial de michelizza, dividido en **5 fases** según la complejidad:

---

### ✅ FASE 1 — Arranque (Boot)
| Paso | Capítulo | Qué construyes | Estado |
|------|----------|----------------|--------|
| 1 | II | Sector de boot que muestra "Hello World" | ✅ Completado |
| 2 | III | Bootloader que carga un kernel desde disco | ✅ Completado |
| 3 | IV | Bootloader que activa el **Modo Protegido** (32-bit) + GDT | ✅ Completado |

---

### ⚙️ FASE 2 — Kernel básico en C
| Paso | Capítulo | Qué construyes | Estado |
|------|----------|----------------|--------|
| 4 | V | Kernel mixto ASM+C, luego kernel 100% en C | ✅ Completado |
| 5 | VI | Kernel que recarga su propia **GDT** | ✅ Completado |
| 6 | VII | Teoría del controlador de interrupciones **8259A** + IDT | ✅ Completado |
| 7 | VIII | Implementación completa de las **interrupciones** | ✅ Completado |
| 8 | IX | Manejo del **teclado** (chipset 8042 + cursor) | ✅ Completado |

---

### 🔀 FASE 3 — Procesos y memoria
| Paso | Capítulo | Qué construyes | Estado |
|------|----------|----------------|--------|
| 9 | X | Primera **tarea de usuario**, TSS, cambio de contexto | ✅ Completado |
| 10 | XI | **System calls** (llamadas al sistema) | ✅ Completado |
| 11 | XII | **Paginación** básica (identity mapping, Page Fault) | ✅ Completado |
| 12 | XIII | Paginación para tareas de usuario, espacios de memoria separados | ✅ Completado |

---

### 🔄 FASE 4 — Multitarea y sistema de archivos
| Paso | Capítulo | Qué construyes | Estado |
|------|----------|----------------|--------|
| 13 | XIV | **Scheduler simple** (round-robin por timer) | ✅ Completado |
| 14 | XV | Scheduler con syscalls **preemptibles**, pila kernel por tarea | ✅ Completado |
| 15 | XVI | Boot con **GRUB** (estándar multiboot) | ✅ Completado |
| 16 | XVII | Gestión completa de **memoria física y virtual** | ✅ Completado |
| 17 | XVIII | Lectura/escritura en **disco IDE** (PIO) | ✅ Completado |
| 18 | XIX | Sistema de archivos **Ext2FS** | ✅ Completado |

---

### 🖥️ FASE 5 — Sistema completo
| Paso | Capítulo | Qué construyes | Estado |
|------|----------|----------------|--------|
| 19 | XX | Cargar y ejecutar binarios **ELF** desde el filesystem | ✅ Completado |
| 20 | XXI | Boot con GRUB en disco IDE particionado | ✅ Completado |
| 21 | XXII | Estructuras para gestión de **archivos** | ✅ Completado |
| 22 | XXIII | Listas enlazadas genéricas (como Linux/FreeBSD) | ✅ Completado |
| 23 | XXIV | Primer **shell** interactivo | ✅ Completado |
| 24 | XXV | **Señales POSIX** (`kill`, `sigaction`, `sigreturn`) | ✅ Completado |

---

### 📚 Anexos disponibles (referencia cuando los necesites)
- **A** — Compilación separada ASM+C en Unix
- **B** — Aritmética en base 16
- **C** — Bochs en modo debug
- **D** — Stack Frames y argumentos en la pila
- **E** — Depurar el kernel con **GDB**
- **F** — Boot con GRUB2

---