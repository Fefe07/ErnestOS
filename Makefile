all: myos

run: myos
	qemu-system-i386 -kernel myos

myos: boot.o kernel.o linker.ld gdt.o idt.o isr.o irq.o
	i686-elf-gcc -T linker.ld -o myos -ffreestanding -O2 -nostdlib irq.o boot.o kernel.o gdt.o idt.o isr.o -lgcc

boot.o: boot.s
	i686-elf-as boot.s -o boot.o

kernel.o: kernel.c
	i686-elf-gcc -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra

gdt.o: gdt.s
	i686-elf-as gdt.s -o gdt.o

isr.o: isr.s
	i686-elf-as isr.s -o isr.o

idt.o: idt.s
	i686-elf-as idt.s -o idt.o

irq.o: irq.s
	i686-elf-as irq.s -o irq.o

clean:
	rm *.o
