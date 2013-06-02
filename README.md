MineAssemble
============

MineAssemble is a tiny bootable Minecraft clone written in x86 assembly. I made
it to learn about writing more complex assembly than the usual "Hello World"
bootloader and to find out how much work it takes to create something like this
without any handholding from a kernel or libraries.

The actual code is designed to be run using QEMU with the `-kernel` flag, but
with some modification it should be possible to turn it into an image that can
be booted using GRUB.

There's also the `reference` directory, which contains C code using the
[SDL library](http://www.libsdl.org/) for a demo with the same functionality. I
figured that starting in assembly right away would be a bit too insane, so I
created the reference code that I would essentially port to assembly after that.

I started writing the reference code with the idea that if it was longer than
150 statements (excluding boilerplate), it wouldn't be worth doing it in
assembly. Like all estimates in the world of programming, this limit turned out
to be a gross underestimate, reaching about 134 lines before adding the texture
code. Nevertheless, since this was originally part of a uni assignment, I still
had a time limit of only about 1 week.

How does it work?
-----------------

TODO