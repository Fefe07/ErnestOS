SRCS_C = $(wildcard *.c)
SRCS_S = $(wildcard *.s)

OBJS = $(SRCS_C:%.c=build/%c.o)
OBJS += $(SRCS_S:%.s=build/%.o)

all: myos

run: myos disk.img
	qemu-system-i386 -kernel myos -k fr -drive format=raw,file=disk.img

myos: $(OBJS)
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib $(OBJS) -lgcc

build/%.o: %.s
	i686-elf-as $< -o $@

build/%c.o: %.c
	i686-elf-gcc -c $< -o $@ -std=gnu99 -ffreestanding -O2 -Wall -Wextra

disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=32
	mke2fs -t ext2 disk.img

clean:
	rm -rf build/*
