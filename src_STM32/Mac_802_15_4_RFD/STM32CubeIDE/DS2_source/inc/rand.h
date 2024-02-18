#ifndef DS2_RAND_H
#define DS2_RAND_H

#include <stdlib.h>

void sample_uniform(int32_t *data, size_t count, int32_t upper_bound);

void sample_normal_from_seed(const uint8_t seed[SEED_BYTES], uint32_t nonce, int32_t mean, int32_t stddev, size_t coeffs_count, int32_t *coeffs);

void sample_normal(int32_t *data, size_t count, int32_t mean, int32_t stddev);

#endif
