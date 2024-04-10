//#include <cmath>
#include "rand.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <stdint.h>

#include "fips202.h"
#include "params.h"

void sample_uniform(int32_t *data, size_t count, int32_t upper_bound) {

#if defined (STM_RNG_H_)
	uint32_t rand_num = 0;
#endif

	for (size_t i = 0; i < count; i++){
        #if defined( USE_SAFE_RANDOM)
        data[i] = arc4random_uniform(upper_bound);
		#elif defined (STM_RNG_H_)
		RNG_GenerateRandomInt(&rand_num);
    	data[i] = upper_bound * (rand_num / (double) RAND_MAX);
        #else
        data[i] = upper_bound * (rand() / (double) RAND_MAX);
        #endif
	}
}

// Transformation from uniform distribution to normal distribution is base on Box-Muller transformation (https://en.wikipedia.org/wiki/Box–Muller_transform).
void uniform_to_normal(double u1, double u2, int32_t mean, int32_t stddev, int32_t *n1, int32_t *n2) {
    *n1 = (sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2) * stddev) + mean;
    *n2 = (sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2) * stddev) + mean;
}

// Expects coeffs_count < N and coeffs_count is even.
void sample_normal_from_seed(const uint8_t seed[SEED_BYTES], uint32_t nonce, int32_t mean, int32_t stddev, size_t coeffs_count, int32_t *coeffs) {
    keccak_state_t state;
    uint8_t data[4 * _N_];
    uint32_t u1, u2;

    keccak_init(&state);

    shake128_absorb(&state, seed, SEED_BYTES);
    shake128_absorb_nonce(&state, nonce++);

    shake128_finalize(&state);

    shake128_squeeze(&state, sizeof(data), data);

    // TODO: do some rejection
    for (size_t i = 0; i < coeffs_count / 2; i++) {
        u1 = data[8 * i] | (((uint32_t) data[8 * i + 1]) << 8) | (((uint32_t) data[8 * i + 2]) << 16) | (((uint32_t) data[8 * i + 3]) << 24);
        u2 = data[8 * i + 4] | (((uint32_t) data[8 * i + 5]) << 8) | (((uint32_t) data[8 * i + 6]) << 16) | (((uint32_t) data[8 * i + 7]) << 24);

        uniform_to_normal((double) u1 / UINT32_MAX, (double) u2 / UINT32_MAX, 0, stddev, &coeffs[2 * i], &coeffs[2 * i + 1]);
    }
}

void sample_normal(int32_t *data, size_t count, int32_t mean, int32_t stddev) {
    double u1, u2, z1, z2;

#if defined(STM_RNG_H_)
    uint32_t rand_num = 0;
#endif

    if ((count & 0x1) == 0x1) {
        #if defined( USE_SAFE_RANDOM)
        u1 = arc4random() / (double) UINT32_MAX;
        u2 = arc4random() / (double) UINT32_MAX;
		#elif defined(STM_RNG_H_)
        RNG_GenerateRandomInt(&rand_num);
        u1 = rand_num / (double) RAND_MAX;
        RNG_GenerateRandomInt(&rand_num);
        u2 = rand_num / (double) RAND_MAX;
        #else
        u1 = rand() / (double) RAND_MAX;
        u2 = rand() / (double) RAND_MAX;
        #endif

        z1 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);

        data[--count] = z1;
    }

    for (size_t i = 0; i < count; i += 2) {
		#if defined( USE_SAFE_RANDOM)
        u1 = arc4random() / (double) UINT32_MAX;
        u2 = arc4random() / (double) UINT32_MAX;
		#elif defined(STM_RNG_H_)
        RNG_GenerateRandomInt(&rand_num);
        u1 = rand_num / (double) RAND_MAX;
        RNG_GenerateRandomInt(&rand_num);
        u2 = rand_num / (double) RAND_MAX;
        #else
        u1 = rand() / (double) RAND_MAX;
        u2 = rand() / (double) RAND_MAX;
        #endif

        z1 = sqrt(-2.0 * log(u1)) * cos(2.0 * M_PI * u2);
        z2 = sqrt(-2.0 * log(u1)) * sin(2.0 * M_PI * u2);

        data[i] = (z1 * stddev) + mean;
        data[i + 1] = (z2 * stddev) + mean;
    }
}
