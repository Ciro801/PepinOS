CC      ?= gcc
LD      ?= ld
BUILD   ?= ../build

CFLAGS_USER = -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic \
              -Wall -mno-sse -mno-mmx

.PHONY: all clean

all: $(BUILD)/hello.elf

$(BUILD)/hello.o: hello.c
	$(CC) $(CFLAGS_USER) -c -o $@ $<

$(BUILD)/hello.elf: $(BUILD)/hello.o link_user.ld
	$(LD) -m elf_i386 -T link_user.ld $(BUILD)/hello.o -o $@

clean:
	rm -f $(BUILD)/hello.o $(BUILD)/hello.elf
