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
	$(BUILD)/vfs.o       \
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
# ── Disco particionado con GRUB ─────────────────────────────────────────────
#
# Estructura del disco (32 MB):
#   Sector 0         : MBR con GRUB boot.img
#   Sectores 1-2047  : GRUB core.img embebido (gap pre-partición)
#   Sector 2048+     : Partición 1 ext2 (kernel.elf + hello.elf + hola.txt)
#
# Herramientas necesarias: grub-mkimage, mtools (mformat, mcopy), fdisk/sfdisk
#
GRUB_LIB   = /usr/lib/grub/i386-pc
GRUB_MODS  = biosdisk part_msdos ext2 normal multiboot
PART_START = 2048    # primer sector de la partición (en sectores de 512 B)
DISK_SECTS = 65536   # 32 MB total

$(BUILD)/disk.img: $(BUILD)/kernel.elf $(BUILD)/hello.elf
	@echo "  [DISK] Creando disco particionado con GRUB..."

	# 1. Imagen de partición ext2 (todo el espacio menos el gap de GRUB)
	dd if=/dev/zero of=$(BUILD)/part.img bs=512 \
	    count=$$(( $(DISK_SECTS) - $(PART_START) )) 2>/dev/null
	mke2fs -b 1024 -t ext2 -F $(BUILD)/part.img 2>/dev/null
	@printf "Hola desde PepinOS Ext2!\n" > /tmp/_pepinos_hola.txt
	printf "mkdir boot\nmkdir boot/grub\nwrite arch/grub.cfg boot/grub/grub.cfg\nwrite $(BUILD)/kernel.elf boot/kernel.elf\nwrite $(BUILD)/hello.elf hello.elf\nwrite /tmp/_pepinos_hola.txt hola.txt\n" | \
	    debugfs -w $(BUILD)/part.img 2>/dev/null || true
	@rm -f /tmp/_pepinos_hola.txt

	# 2. Disco vacío de 32 MB con tabla de particiones MBR
	dd if=/dev/zero of=$@ bs=512 count=$(DISK_SECTS) 2>/dev/null
	printf "$(PART_START),$$(( $(DISK_SECTS) - $(PART_START) )),83,*\n" | \
	    sfdisk --no-reread $@ 2>/dev/null

	# 3. Incrustar la partición en el disco al offset correcto
	dd if=$(BUILD)/part.img of=$@ bs=512 seek=$(PART_START) conv=notrunc 2>/dev/null

	# 4. Construir GRUB core.img con los módulos necesarios
	grub-mkimage -O i386-pc -o $(BUILD)/grub_core.img \
	    -p '(hd0,msdos1)/boot/grub' \
	    $(GRUB_MODS) 2>/dev/null

	# 5. Incrustar GRUB: boot.img en MBR + core.img en sectores 1..N
	cp $(GRUB_LIB)/boot.img $(BUILD)/grub_boot.img
	# Parchar boot.img: dirección LBA del core.img = sector 1
	printf '\x01\x00\x00\x00' | dd of=$(BUILD)/grub_boot.img \
	    bs=1 seek=92 conv=notrunc 2>/dev/null
	# Solo 446 bytes de boot code — preserva la tabla de particiones de sfdisk
	dd if=$(BUILD)/grub_boot.img of=$@ bs=1 count=446 conv=notrunc 2>/dev/null
	dd if=$(BUILD)/grub_core.img of=$@ bs=512 seek=1 conv=notrunc 2>/dev/null

	@echo "  [DISK] Listo: $@"

# Arranque real desde GRUB (sin -kernel)
run: all $(BUILD)/disk.img
	qemu-system-i386 \
	    -drive file=$(BUILD)/disk.img,format=raw,if=ide \
	    -k es

# -d int muestra todas las interrupciones en la consola de QEMU
debug: all $(BUILD)/disk.img
	qemu-system-i386 \
	    -drive file=$(BUILD)/disk.img,format=raw,if=ide \
	    -k es -d int 2>&1 | head -200

clean:
	rm -f $(BUILD)/*.o \
	      $(BUILD)/multiboot.o \
	      $(BUILD)/kernel.elf \
	      $(BUILD)/disk.img \
	      $(BUILD)/part.img \
	      $(BUILD)/grub_core.img \
	      $(BUILD)/grub_boot.img \
	      $(BUILD)/hello.elf \
	      $(BUILD)/hello.o
