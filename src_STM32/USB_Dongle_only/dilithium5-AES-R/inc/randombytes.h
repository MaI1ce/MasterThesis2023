#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include "stm32wbxx_hal.h"

void randombytes(RNG_HandleTypeDef *hrng, uint8_t *out, size_t outlen);

#endif
