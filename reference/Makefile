CC = gcc
CFLAGS = -O3 -Wall -Werror

bin/mineassemble: bin bin/mineassemble.o bin/textures.o
	$(CC) $(CFLAGS) -o bin/mineassemble bin/mineassemble.o bin/textures.o -lSDL -lm

bin/mineassemble.o: src/mineassemble.c
	$(CC) $(CFLAGS) -c -o bin/mineassemble.o src/mineassemble.c -std=c99

bin/textures.o: src/textures.c
	$(CC) $(CFLAGS) -c -o bin/textures.o src/textures.c

bin:
	mkdir -p bin

test:
	bin/mineassemble

clean:
	rm -rf bin