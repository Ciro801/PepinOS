CC      ?= gcc
BUILD   ?= ../build
INCLUDES ?= -I../lib -I../kernel -I../arch -I../mm -I../drivers -I../process
CFLAGS  ?= -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall $(INCLUDES)

OBJS = $(BUILD)/paging.o $(BUILD)/pmm.o $(BUILD)/vmm.o

.PHONY: all clean

all: $(OBJS)

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
