CC      ?= gcc
BUILD   ?= ../build
INCLUDES ?= -I../lib -I../kernel -I../arch -I../mm -I../drivers -I../process
CFLAGS  ?= -m32 -ffreestanding -fno-stack-protector -nostdlib -fno-pic -Wall $(INCLUDES)

# scheduler.o, elf.o, signal.o se añaden en pasos futuros
OBJS = $(BUILD)/task.o $(BUILD)/task_user.o $(BUILD)/syscall.o

.PHONY: all clean

all: $(OBJS)

$(BUILD)/%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS)
