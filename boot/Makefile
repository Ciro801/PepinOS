NASM  ?= nasm
BUILD ?= ../build

.PHONY: all clean

all: $(BUILD)/multiboot.o

$(BUILD)/multiboot.o: multiboot.asm
	$(NASM) -f elf -o $@ $<

clean:
	rm -f $(BUILD)/multiboot.o
