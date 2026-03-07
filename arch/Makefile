CC      ?= gcc
NASM    ?= nasm
BUILD   ?= ../build
INCLUDES ?= -I../lib -I../kernel -I../arch -I../mm -I../drivers -I../process
CFLAGS  ?= -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall $(INCLUDES)

OBJS = $(BUILD)/gdt.o $(BUILD)/tss.o $(BUILD)/idt.o $(BUILD)/idt_asm.o

.PHONY: all clean

all: $(OBJS)

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

$(BUILD)/%.o: %.asm
	$(NASM) -f elf -o $@ $<

clean:
	rm -f $(OBJS)
