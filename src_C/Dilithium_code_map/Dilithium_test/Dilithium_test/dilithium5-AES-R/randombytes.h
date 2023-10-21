#ifndef RANDOMBYTES_H
#define RANDOMBYTES_H

#include <stddef.h>
#include <stdint.h>

extern uint8_t const_base;

void randombytes(uint8_t *out, size_t outlen);

void randombytes2(uint8_t* out, size_t outlen);

#endif
