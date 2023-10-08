
#include <stdint.h>
#include <stdlib.h>
#include "randombytes.h"


static uint32_t LFSR32() {
	// x^32 + x^22 + x^2 + x + 1
	static uint32_t seed = 0xA5A5A5A5;
	seed = (seed << 1) | (((seed >> 31) & 1) ^ ((seed >> 21) & 1) ^ ((seed >> 1) & 1) ^ ((seed >> 0) & 1) ^ 1);
	return seed;
}

void randombytes( uint8_t *out, size_t outlen) {
	// TODO - LFSR PLACEHOLDER
	uint32_t * ptr = (uint32_t *)out;
	int i = 0;
	int j = 0;
	for(; i < outlen / 4; i++){
		ptr[i] = LFSR32();
	}
	uint32_t lfsr_num = LFSR32();
	for(; j < outlen % 4; j++){
		out[i*4+j] = (uint8_t)(lfsr_num >> (j*8));
	}
}

