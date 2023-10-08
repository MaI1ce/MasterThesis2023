
#include <stdint.h>
#include <stdlib.h>
#include "randombytes.h"


void randombytes(RNG_HandleTypeDef *hrng, uint8_t *out, size_t outlen) {
	uint32_t rand_int = 0;
	uint8_t *rand_int_byte = (uint8_t*)&rand_int;
	uint32_t *int_out = (uint32_t*)out;
	int i = 0;
	int j = 0;

	for(; i < outlen/4; i++){
		HAL_RNG_GenerateRandomNumber(hrng, &int_out[i]);
	}

	int rem = outlen % 4;
	if (rem) {
		HAL_RNG_GenerateRandomNumber(hrng, &rand_int);
		for(; j < rem; j++){
			out[i+j] = rand_int_byte[j];
		}
	}
}

