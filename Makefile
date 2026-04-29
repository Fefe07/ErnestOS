all: myos

run: myos
	qemu-system-i386 -kernel myos

myos: boot.o kernel.o linker.ld gdt.o
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib boot.o kernel.o gdt.o -lgcc

boot.o: boot.s
	i686-elf-as boot.s -o boot.o

kernel.o: kernel.c
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

gdt.o: gdt.s
	i686-elf-as gdt.s -o gdt.o

clean:
	rm *.o
