CC   = gcc
NASM = nasm
LD   = ld

BUILD = build

# Rutas absolutas para que los sub-makes las usen cuando se exportan
INCLUDES = -I$(CURDIR)/lib    \
           -I$(CURDIR)/kernel \
           -I$(CURDIR)/arch   \
           -I$(CURDIR)/mm     \
           -I$(CURDIR)/drivers \
           -I$(CURDIR)/process

CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall $(INCLUDES)

export CC NASM CFLAGS BUILD

# multiboot.o va primero: la sección .multiboot debe quedar en los
# primeros 8KB del binario para que GRUB encuentre la cabecera.
OBJS = \
	$(BUILD)/multiboot.o \
	$(BUILD)/kernel.o    \
	$(BUILD)/gdt.o       \
	$(BUILD)/tss.o       \
	$(BUILD)/idt.o       \
	$(BUILD)/idt_asm.o   \
	$(BUILD)/keyboard.o  \
	$(BUILD)/syscall.o   \
	$(BUILD)/paging.o    \
	$(BUILD)/task.o      \
	$(BUILD)/task_user.o \
	$(BUILD)/scheduler.o \
	$(BUILD)/screen.o

.PHONY: all run clean debug

all: $(BUILD)/kernel.elf

# ── Compilación: llamar a cada subcarpeta en orden ─────────────────────────
$(BUILD)/kernel.elf: _compile $(OBJS)
	$(LD) -m elf_i386 -T arch/linker.ld \
	      $(OBJS) -o $(BUILD)/kernel.elf

.PHONY: _compile
_compile:
	@mkdir -p $(BUILD)
	$(MAKE) -C boot    BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C lib     BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C kernel  BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C arch    BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C mm      BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C drivers BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C process BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C fs      BUILD=$(CURDIR)/$(BUILD)
	$(MAKE) -C user    BUILD=$(CURDIR)/$(BUILD)

# ── Targets ────────────────────────────────────────────────────────────────
# QEMU implementa Multiboot nativamente con -kernel: no se necesita GRUB real
run: all
	qemu-system-i386 \
	    -kernel $(BUILD)/kernel.elf \
	    -k es

# -d int muestra todas las interrupciones en la consola de QEMU
debug: all
	qemu-system-i386 \
	    -kernel $(BUILD)/kernel.elf \
	    -k es -d int 2>&1 | head -200

clean:
	rm -f $(BUILD)/*.o \
	      $(BUILD)/multiboot.o \
	      $(BUILD)/kernel.elf
