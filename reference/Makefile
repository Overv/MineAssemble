CC = gcc
CFLAGS = -O3 -Wall -Werror

bin/mineassemble: src/mineassemble.c bin
	$(CC) $(CFLAGS) -o bin/mineassemble src/mineassemble.c -lSDL -lm -std=c99

bin:
	mkdir -p bin

test:
	bin/mineassemble

clean:
	rm -rf bin