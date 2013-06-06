bin/mineassemble.bin: bin src/link.ld bin/init.o bin/interrupts.o bin/vga.o bin/main.o
	ld -m elf_i386 -T src/link.ld -o bin/mineassemble.bin bin/init.o bin/interrupts.o bin/vga.o bin/main.o

bin/%.o: src/%.asm
	nasm -felf -o $@ $<

bin:
	mkdir -p bin

test:
	qemu-system-i386 -kernel bin/mineassemble.bin

clean:
	rm -rf bin