CC      ?= gcc
BUILD   ?= ../build
INCLUDES ?= -I../lib
CFLAGS  ?= -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall $(INCLUDES)

# string.o y list.o se añaden en pasos futuros
OBJS =

.PHONY: all clean

all: $(OBJS)

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
