CC      ?= gcc
BUILD   ?= ../build
INCLUDES ?= -I../lib -I../kernel -I../arch -I../mm -I../drivers -I../process -I../fs
CFLAGS  ?= -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall $(INCLUDES)

OBJS = $(BUILD)/ext2.o

.PHONY: all clean

all: $(OBJS)

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
