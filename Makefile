CC=i686-pc-elf-gcc
LD=i686-pc-elf-ld
ASM=nasm

CFLAGS:=-std=c99 -MMD
CFLAGS+=-m32
CFLAGS+=-g -ggdb
CFLAGS+=-O3 -ffast-math
CFLAGS+=-ffreestanding -nostdlib -nostdinc -fno-builtin -nostartfiles -nodefaultlibs -fno-exceptions -fno-stack-protector -static -fno-pic

LDFLAGS=-m elf_i386
LDLIBS=

ASMFLAGS=-felf -isrc/
ASMFLAGS+=-g

CSRCS=$(wildcard src/*.c)
ASMSRCS=$(wildcard src/*.asm)
OBJS=$(CSRCS:.c=.o) $(ASMSRCS:.asm=.o)
DEPS=$(OBJS:.o=.d)

.PHONY: all clean

all: mineassemble.bin mineassemble.elf

# NOTE: linker script must be first dependency
mineassemble.%: src/%.ld $(OBJS)
	$(LD) $(LDFLAGS) -o $@ -T $^ $(LDLIBS)

%.o: %.asm
	$(ASM) $(ASMFLAGS) -MD $(@:.o=.d) -o $@ $<

-include $(DEPS)

# Test in QEMU

test: mineassemble.bin
	qemu-system-i386 -kernel mineassemble.bin

.PHONY: test

# Target for producing ISO image

iso: mineassemble.iso
mineassemble.iso: mineassemble.bin
	cp mineassemble.bin iso/boot/mineassemble.bin
	grub-mkrescue -o mineassemble.iso iso

.PHONY: iso

# Clean up

clean:
	rm -f $(OBJS)
	rm -f $(DEPS)
	rm -f mineassemble.bin
	rm -f mineassemble.elf
	rm -f mineassemble.iso
	rm -f iso/boot/mineassemble.bin
