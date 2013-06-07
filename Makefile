bin/mineassemble.bin: bin src/link.ld bin/init.o bin/interrupts.o bin/vga.o bin/reference.o
	ld -m elf_i386 -T src/link.ld -o bin/mineassemble.bin bin/init.o bin/interrupts.o bin/vga.o bin/reference.o

bin/reference.o: src/reference.c
	gcc -m32 -c -o bin/reference.o src/reference.c -std=c99 -ffreestanding -O3

bin/%.o: src/%.asm
	nasm -felf -o $@ $<

bin:
	mkdir -p bin

test:
	qemu-system-i386 -kernel bin/mineassemble.bin

iso: mineassemble.iso
mineassemble.iso: bin/mineassemble.bin
	cp bin/mineassemble.bin iso/boot/mineassemble.bin
	grub-mkrescue -o mineassemble.iso iso

clean:
	rm -rf bin
