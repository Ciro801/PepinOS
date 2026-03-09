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
           -I$(CURDIR)/process \
           -I$(CURDIR)/fs

CFLAGS = -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall \
         -mno-sse -mno-mmx \
         $(INCLUDES)

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
	$(BUILD)/ide.o       \
	$(BUILD)/syscall.o   \
	$(BUILD)/paging.o    \
	$(BUILD)/pmm.o       \
	$(BUILD)/vmm.o       \
	$(BUILD)/ext2.o      \
	$(BUILD)/elf.o       \
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
# Crear imagen de disco ext2 de 2MB con un archivo de prueba
$(BUILD)/disk.img: $(BUILD)/hello.elf
	dd if=/dev/zero of=$(BUILD)/disk.img bs=1k count=2048 2>/dev/null
	mkfs.ext2 -b 1024 $(BUILD)/disk.img 2>/dev/null
	@printf "Hola desde PepinOS Ext2!\n" > /tmp/_pepinos_test.txt
	@printf "write /tmp/_pepinos_test.txt hola.txt\nwrite $(BUILD)/hello.elf hello.elf\n" | \
	    debugfs -w $(BUILD)/disk.img 2>/dev/null || true
	@rm -f /tmp/_pepinos_test.txt

# QEMU implementa Multiboot nativamente con -kernel: no se necesita GRUB real
run: all $(BUILD)/disk.img
	qemu-system-i386 \
	    -kernel $(BUILD)/kernel.elf \
	    -drive file=$(BUILD)/disk.img,format=raw,if=ide \
	    -k es

# -d int muestra todas las interrupciones en la consola de QEMU
debug: all $(BUILD)/disk.img
	qemu-system-i386 \
	    -kernel $(BUILD)/kernel.elf \
	    -drive file=$(BUILD)/disk.img,format=raw,if=ide \
	    -k es -d int 2>&1 | head -200

clean:
	rm -f $(BUILD)/*.o \
	      $(BUILD)/multiboot.o \
	      $(BUILD)/kernel.elf \
	      $(BUILD)/disk.img \
	      $(BUILD)/hello.elf \
	      $(BUILD)/hello.o
