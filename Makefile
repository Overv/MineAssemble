bin/mineassemble.bin: bin src/link.ld bin/init.o
	ld -m elf_i386 -T src/link.ld -o bin/mineassemble.bin bin/init.o

bin/init.o: src/init.asm
	nasm -felf -o bin/init.o src/init.asm

bin:
	mkdir -p bin

test:
	qemu-system-i386 -kernel bin/mineassemble.bin

clean:
	rm -rf bin