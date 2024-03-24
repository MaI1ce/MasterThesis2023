#ifndef DS2_FIPS202_H
#define DS2_FIPS202_H

#include <stddef.h>
#include <stdint.h>

#define SHAKE128_RATE 168
#define SHAKE256_RATE 136

typedef struct {
  uint64_t s[25];
  uint8_t pos;
} keccak_state_t;

void keccak_init(keccak_state_t *state);


void shake128_absorb(keccak_state_t *state, const uint8_t *data, size_t data_len);

void shake128_absorb_nonce(keccak_state_t *state, uint32_t nonce);

void shake128_finalize(keccak_state_t *state);

void shake128_squeeze(keccak_state_t *state, size_t data_len, uint8_t *data);


void shake256_absorb(keccak_state_t *state, const uint8_t *data, size_t data_len);

void shake256_absorb_nonce(keccak_state_t *state, uint32_t nonce);

void shake256_finalize(keccak_state_t *state);

void shake256_squeeze(keccak_state_t *state, size_t data_len, uint8_t *data);

#endif
