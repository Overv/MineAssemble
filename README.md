MineAssemble
============

MineAssemble is a tiny bootable Minecraft clone written in x86 assembly. I made
it to learn about writing more complex assembly than the usual "Hello World"
bootloader and to find out how much work it takes to create something like this
without any handholding from a kernel or libraries.

The actual code is designed to be run using QEMU with the `-kernel` flag to
allow for easy testing. The makefile also includes a target to produce an .iso
image for a CD or DVD.

There's also the *reference* directory, which contains C code using the
[SDL library](http://www.libsdl.org/) for a demo with the same functionality. I
figured that starting in assembly right away would be a bit too insane, so I
created the reference code that I would essentially port to assembly after that.

I started writing the reference code with the idea that if it was longer than
150 statements (excluding boilerplate), it wouldn't be worth doing it in
assembly. Like all estimates in the world of programming, this limit turned out
to be a gross underestimate, reaching about 134 lines before adding the texture
code. Nevertheless, since this was originally part of a uni assignment, I still
tried to finish it within the 1 week of time I had left.

After completing the reference code, I wrote the kernel boilerplate code
(setting up VGA, interrupts, etc.) and changed the reference C code to work with
this. Then I began slowly porting everything to handwritten assembly.

Usage
-----

### QEMU

To run the game with QEMU, simply run `make test` This is a quick and easy way
to play around with it.

### Virtual machine

If you want to use virtualization software like VirtualBox, you can produce an
.iso image with `make iso` and mount it.

You can also burn this image to a CD or DVD, but that is rather wasteful. Use
the USB stick method to try it on real hardware unless it really isn't an option
for some reason.

### USB stick

Booting from an USB stick is an excellent way to try it on real hardware, but
does involve a little bit more work. Note that this process will remove all data
currently on the USB stick. Also, make sure to get the drive name right or you
might accidentally wipe your hard drive!

1. Format your USB stick to FAT32 with 1 MB free space preceding.
2. Mount it using `mount /dev/sdx1 /mnt` where `sdx` is the drive name.
3. Turn it into a GRUB rescue disk with `grub-install --no-floppy --root-directory=/mnt /dev/sdx`.
4. Run `make iso` and copy the contents of the *iso* directory to the USB stick.
5. Unmount with `umount -l /dev/sdx1`.

Now reboot your PC and boot from USB.

Explanation
-----------

TODO

License
-------

This project is licensed under the GPL v2 license.

Some derived work with compatible licensing is also included:

- **init.asm**, **interrupts.asm** - Derived from [code by Maarten de Vries and Maurice Bos](https://github.com/m-ou-se/bootlib) (licensed under the MIT license)
- **vga.asm** - Derived from [code by Christoffer Bubach](http://bos.asmhackers.net/docs/vga_without_bios/snippet_5/vga.php) (licensed under the GPL license)

Derived work here means that the code was adapted to fit the requirements of this project.