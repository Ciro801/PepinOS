NASM  ?= nasm
BUILD ?= ../build

.PHONY: all clean

all: $(BUILD)/bootsect

$(BUILD)/bootsect: bootsect.asm
	$(NASM) -f bin -o $@ $<

clean:
	rm -f $(BUILD)/bootsect
