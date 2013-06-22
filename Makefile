# Default compiler, assembler, and linker
CC := gcc
AS := nasm
LD := ld

# Append required CFLAGS, LDFLAGS, and ASFLAGS to those specified on the command line.
override CFLAGS+=-m32 -c -g -std=c99 -ffreestanding -Ofast -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-stack-protector -static -fno-pic

override LDFLAGS+=-m elf_i386 -T src/link.ld

override ASFLAGS+=-felf -isrc/

OBJECTS = bin/init.o bin/interrupts.o bin/vga.o bin/main.o bin/reference.o bin/textures.o bin/cmath.o bin/splash.o bin/world.o bin/player.o bin/input.o bin/graphics.o bin/globals.o

# Default build option, also allows `make clean all` (rather
all: iso

# Build flat binary

bin/mineassemble.bin: bin src/link.ld ${OBJECTS}
	${LD} ${LDFLAGS} -o bin/mineassemble.bin ${OBJECTS}

bin/reference.o: src/reference.c
	${CC} ${CFLAGS} -o bin/reference.o src/reference.c

bin/%.o: src/%.asm
	${AS} ${ASFLAGS} -o $@ $<

bin:
	mkdir -p bin

# Test in QEMU

test: bin/mineassemble.bin
	qemu-system-i386 -kernel bin/mineassemble.bin

# Target for producing ISO image

iso: mineassemble.iso
mineassemble.iso: bin/mineassemble.bin
	cp bin/mineassemble.bin iso/boot/mineassemble.bin
	grub-mkrescue -o mineassemble.iso iso


# Clean up

clean:
	rm -rf bin

.PHONY: iso test all
