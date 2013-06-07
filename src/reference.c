#include <stdint.h>

void main() {
	uint8_t* vga = (uint8_t*) 0xa0000;

	for (int i = 0; i < 320 * 200; i++) {
		vga[i] = 0x01;
	}
}