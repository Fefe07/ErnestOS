SRCS_C = $(wildcard *.c)
SRCS_S = $(wildcard *.s)

OBJS = $(SRCS_C:%.c=build/%c.o)
OBJS += $(SRCS_S:%.s=build/%.o)

all: myos

run: myos
	qemu-system-i386 -kernel myos -k fr

myos: $(OBJS)
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib $(OBJS) -lgcc

build/%.o: %.s
	i686-elf-as $< -o $@

build/%c.o: %.c
	i686-elf-gcc -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

clean:
	rm -rf build/*
	rm myos
