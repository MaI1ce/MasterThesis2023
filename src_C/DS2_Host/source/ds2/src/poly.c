/* Based on the public domain Dilithium reference implementation at https://github.com/pq-crystals/dilithium
 * by Léo Ducas, Eike Kiltz, Tancrède Lepoint, Vadim Lyubashevsky, Gregor Seiler, Peter Schwabe and Damien Stehlé.
 */
//#include <cmath>
#include "poly.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>


#include "commit.h"
#include "fips202.h"
#include "ntt.h"
#include "params.h"
#include "rand.h"
#include "reduce.h"


//#include "elapsed_time.h"
#ifndef ELAPSED_TIME_H_
#define TB_START(i)
#define TB_END(i)
#endif

void poly_add(const poly_t *a, const poly_t *b, size_t polys_count, poly_t *c) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            c[i].coeffs[j] = a[i].coeffs[j] + b[i].coeffs[j];
}

void poly_sub(const poly_t *a, const poly_t *b, size_t polys_count, poly_t *c) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            c[i].coeffs[j] = a[i].coeffs[j] - b[i].coeffs[j];
}

void poly_mul_pointwise(const poly_t *a, const poly_t *b, size_t polys_count, poly_t *c) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            c[i].coeffs[j] = montgomery_reduce((int64_t) a[i].coeffs[j] * b->coeffs[j]);
}

void poly_const_mul(const poly_t *a, int32_t constant, size_t polys_count, poly_t *c) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            c[i].coeffs[j] = (constant * a[i].coeffs[j]);
}

void poly_mul_acc(const poly_t *a, const poly_t *b, size_t polys_count, poly_t *c) {
    for (size_t i = 0; i < _N_; i++)
        c->coeffs[i] = montgomery_reduce((int64_t) a[0].coeffs[i] * b[0].coeffs[i]);

    for (size_t i = 1; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            c->coeffs[j] += montgomery_reduce((int64_t) a[i].coeffs[j] * b[i].coeffs[j]);
}

void poly_addq(poly_t *poly, size_t polys_count) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            poly[i].coeffs[j] = addq(poly[i].coeffs[j]);
}

void poly_reduce(poly_t *poly, size_t polys_count) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            poly[i].coeffs[j] = reduce32(poly[i].coeffs[j]);
}

void poly_freeze(poly_t *poly, size_t polys_count) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            poly[i].coeffs[j] = freeze(poly[i].coeffs[j]);
}

void poly_center(poly_t *poly, size_t polys_count) {
    for (size_t i = 0; i < polys_count; i++)
        for (size_t j = 0; j < _N_; j++)
            poly[i].coeffs[j] = center(poly[i].coeffs[j]);
}

void poly_ntt(poly_t *poly, size_t polys_count) {
    for (size_t i = 0; i < polys_count; i++)
        ntt(poly[i].coeffs);
}

void poly_invntt_tomont(poly_t *poly, size_t polys_count) {
    for (size_t i = 0; i < polys_count; i++)
        invntt_tomont(poly[i].coeffs);
}

void poly_product(const poly_t a[_K_][_L_], const poly_t b[_L_], poly_t c[_K_]) {
    for (size_t i = 0; i < _K_; i++)
        poly_mul_acc(a[i], b, _L_, &c[i]);
}

void poly_product2(const poly_t a[2][TC_COLS], const poly_t b[TC_COLS], poly_t c[2]) {
    for (size_t i = 0; i < 2; i++)
        poly_mul_acc(a[i], b, TC_COLS, &c[i]);
}

void poly_copy(const poly_t *poly, size_t polys_count, poly_t *copy) {
    for (size_t i = 0; i < polys_count; i++)
        copy[i] = poly[i];
}

void poly_uniform(const uint8_t seed[SEED_BYTES], size_t polys_count, size_t nonce, poly_t *poly) {
    TB_START(POLY_UNIFORM_INDEX)

    keccak_state_t state;
    uint8_t data[24 * _N_];
    uint16_t count;
    int32_t coeff;

    for (size_t i = 0; i < polys_count; i++) {
        keccak_init(&state);

        shake128_absorb(&state, seed, SEED_BYTES);
        shake128_absorb_nonce(&state, (uint32_t) i+nonce);

        shake128_finalize(&state);

        count = 0;

        do {
            shake128_squeeze(&state, sizeof(data), data);

            for (size_t j = 0; j < _N_; j++) {
                coeff = ((data[3 * j] << 16) | (data[3 * j + 1] << 8) | data[3 * j]) & 0x7fffff;

                if (coeff < _Q_)
                    poly[i].coeffs[count++] = coeff;

                if (count == _N_)
                    break;
            }
        } while (count != _N_);
    }

    TB_END(POLY_UNIFORM_INDEX)
}

void poly_eta(const uint8_t seed[SEED_BYTES], uint32_t nonce, size_t polys_count, poly_t *poly) {
    keccak_state_t state;
    uint8_t data[_N_];
    uint16_t count;
    uint8_t t0, t1;

    for (size_t i = 0; i < polys_count; i++) {
        keccak_init(&state);

        shake128_absorb(&state, seed, SEED_BYTES);
        shake128_absorb_nonce(&state, nonce++);

        shake128_finalize(&state);

        count = 0;

        do {
            shake128_squeeze(&state, sizeof(data), data);

            for (size_t j = 0; j < _N_; j++) {
                t0 = data[j] & 0x0f;
                t1 = (data[j] >> 4) & 0x0f;

                #if ETA == 2
                    if (t0 < 15)
                        poly[i].coeffs[count++] = 2 - t0 + (205 * t0 >> 10) * 5;
                    if (t1 < 15 && count < _N_)
                        poly[i].coeffs[count++] = 2 - t1 + (205 * t1 >> 10) * 5;
                #elif ETA == 4
                    if (t0 < 9)
                        poly[i].coeffs[count++] = 4 - t0;
                    if (t1 < 9 && count < _N_)
                        poly[i].coeffs[count++] = 4 - t1;
                #elif ETA == 5
                    if (t0 < 11)
                        poly[i].coeffs[count++] = 5 - t0;
                    if (t1 < 11 && count < _N_)
                        poly[i].coeffs[count++] = 5 - t1;
                #endif

                if (count == _N_)
                    break;
            }
        } while (count != _N_);
    }
}

// Transformation from uniform distribution to normal distribution is base on Box-Muller transformation (https://en.wikipedia.org/wiki/Box–Muller_transform).
void poly_normal(const uint8_t seed[SEED_BYTES], uint32_t nonce, uint32_t stddev, size_t polys_count, poly_t *poly) {
    for (size_t i = 0; i < polys_count; i++)
        sample_normal_from_seed(seed, nonce++, 0, stddev, _N_, poly[i].coeffs);
}

void poly_challenge(const uint8_t seed[SEED_BYTES], poly_t *poly) {
    keccak_state_t state;
    uint8_t data[SHAKE256_RATE];
    uint64_t signs = 0;
    size_t pos = 0;
    uint8_t b;

    keccak_init(&state);

    shake256_absorb(&state, seed, SEED_BYTES);
    shake256_finalize(&state);
    shake256_squeeze(&state, SHAKE256_RATE, data);

    for (size_t i = 0; i < 8; i++)
        signs |= (uint64_t) data[pos++] << (8 * i);

    for (size_t i = _N_ - KAPPA; i < _N_; i++) {
        do {
            if (pos == SHAKE256_RATE) {
                shake256_squeeze(&state, SHAKE256_RATE, data);
                pos = 0;
            }

            b = data[pos++];
        } while(b > i);

        poly->coeffs[i] = poly->coeffs[b];
        poly->coeffs[b] = 1 - 2 * (signs & 1);

        signs >>= 1;
    }
}

void poly_make_hint(const poly_t *w1, size_t polys_count, poly_t *h) {
    poly_decompose(h, polys_count, NULL);

    for (size_t i = 0; i < polys_count; i++) {
        for (size_t j = 0; j < _N_; j++) {
            int32_t hint = (w1[i].coeffs[j] - h[i].coeffs[j]) & ((1 << 9) - 1);
            // hint -= (1 << 9) & (((1 << 8) - hint) >> 31);

            h[i].coeffs[j] = hint;
        }
    }
}

void poly_use_hint(const poly_t *h, size_t polys_count, poly_t *w1) {
    poly_decompose(w1, polys_count, NULL);

    for (size_t i = 0; i < polys_count; i++) {
        for (size_t j = 0; j < _N_; j++) {
            w1[i].coeffs[j] = (w1[i].coeffs[j] + h[i].coeffs[j]) & ((1 << 9) - 1);
            w1[i].coeffs[j] += (1 << 9) & (w1[i].coeffs[j] >> 31);
        }
    }
}

void poly_commit(const poly_t *poly, size_t polys_count, const poly_t ck[][TC_COLS], poly_t r[][TC_COLS], poly_t f[][2]) {
    TB_START(POLY_COMMIT_INDEX)
    int8_t bound_exceeded;

    for (size_t i = 0; i < polys_count; i++) {
        do {
            sample_normal((int32_t*) r[i], _N_ * TC_COLS, 0, TC_S);

            bound_exceeded = commit(&poly[i], ck, r[i], TC_B, f[i]);
        } while (bound_exceeded != 0);
    }

    TB_END(POLY_COMMIT_INDEX)
}

int8_t poly_commit2(const poly_t *poly, size_t polys_count, const poly_t ck[][TC_COLS], const poly_t r[][TC_COLS], poly_t f[][2]) {
    int8_t bound_exceeded = 0;

    for (size_t i = 0; i < polys_count; i++)
        bound_exceeded |= commit(&poly[i], ck, r[i], N_PARTIES_SQRT * TC_B, f[i]);

    return bound_exceeded;
}

uint8_t poly_open(const poly_t *poly, size_t polys_count, const poly_t ck[][TC_COLS], const poly_t r[][TC_COLS], const poly_t f[][2]) {
    TB_START(POLY_OPEN_INDEX)
    int8_t bound_exceeded = 0;

    poly_t _f[2];

    for (size_t i = 0; i < polys_count; i++) {
        bound_exceeded |= commit(&poly[i], ck, r[i], TC_B, _f);

        poly_freeze(_f, 2);

        for (size_t j = 0; j < _N_; j++) {
            if (f[i][0].coeffs[j] != _f[0].coeffs[j] || f[i][1].coeffs[j] != _f[1].coeffs[j]) {
                //printf("%d, %x, %x, %x, %x\n", j, f[i][0].coeffs[j], _f[0].coeffs[j], f[i][1].coeffs[j], _f[1].coeffs[j]);
                return 0;
            }
        }
    }

    TB_END(POLY_OPEN_INDEX)

    return !bound_exceeded;
}

void poly_power2round(poly_t *r1, size_t polys_count, poly_t *r0) {
    for (size_t i = 0; i < polys_count; i++) {
        for (size_t j = 0; j < _N_; j++) {
            r0[i].coeffs[j] = (r1[i].coeffs[j] & ((1 << _D_) - 1));
            r0[i].coeffs[j] -= (1 << _D_) & (((1 << (_D_ - 1)) - r0[i].coeffs[j]) >> 31);
            r1[i].coeffs[j] = (r1[i].coeffs[j] - r0[i].coeffs[j]) >> _D_;
        }
    }
}

void poly_decompose(poly_t *r1, size_t polys_count, poly_t *r0) {
   for (size_t i = 0; i < polys_count; i++) {
        for (size_t j = 0; j < _N_; j++) {
            // TODO: do this more effectively for given ALPHA
            int32_t _r1 = r1[i].coeffs[j];
            int32_t _r0 = _r1 % ALPHA;

            if (_r0 > (ALPHA >> 1))
                _r0 -= ALPHA;

            if (_r1 - _r0 == _Q_ - 1) {
                _r0 -= 1;
                _r1 = 0;
            } else
                _r1 = (_r1 - _r0) / ALPHA;

            r1[i].coeffs[j] = _r1;
            if (r0 != NULL)
                r0[i].coeffs[j] = _r0;
        }
    }
}

uint8_t poly_check_norm(const poly_t *poly, size_t polys_count, double bound) {
    for (size_t i = 0; i < polys_count; i++) {
        uint64_t norm = 0;

        for (size_t j = 0; j < _N_; j++) {
            int32_t coeff = poly[i].coeffs[j];

            norm += (int64_t) coeff * coeff;
        }

        // printf("BOUND CHECK: %f, %llu, %f\n", sqrt((double) norm), norm, bound);
        if (sqrt((double) norm) > bound) {
            // printf("failed check norm\n");

            return 0;
        }
    }

    return 1;
}

uint8_t poly_reject(const poly_t z1[_L_], const poly_t z2[_K_], const poly_t cs1[_L_], const poly_t cs2[_K_]) {
    #ifdef USE_SAFE_RANDOM
    double u = arc4random() / (double) UINT32_MAX;
	#elif defined (STM_RNG_H_)
    uint32_t rand_num = 0;
	RNG_GenerateRandomInt(&rand_num);
	double u = rand_num / (double) RAND_MAX;
    #else
    double u = rand() / (double) RAND_MAX;
    #endif
    double x = 0;

    for (size_t i = 0; i < _L_; i++)
        for (size_t j = 0; j < _N_; j++)
            x += (-2.0 * z1[i].coeffs[j] + cs1[i].coeffs[j]) * cs1[i].coeffs[j];

    for (size_t i = 0; i < _K_; i++)
        for (size_t j = 0; j < _N_; j++)
            x += (-2.0 * z2[i].coeffs[j] + cs2[i].coeffs[j]) * cs2[i].coeffs[j];

    x /= 2.0 * SIGMA * SIGMA;

    // printf("REJECT CHECK: %f, %f, %f, %f\n", u, (exp(x) / _M_), x, exp(x));

    return u > (exp(x) / _M_);
}

void poly_pack(uint8_t valid_bits, const poly_t *poly, size_t polys_count, uint8_t *data) {
    uint8_t offset = 0;
    size_t index = 0;

    for (size_t i = 0; i < polys_count; i++) {
        for (size_t j = 0; j < _N_; j++) {
            size_t k;
            int32_t coeff = poly[i].coeffs[j];

            for (k = 0; k < (valid_bits + offset) / 8; k++) {
                data[index] &= (0xff >> (8 - (k == 0 ? offset : 0)));
                data[index++] |= (coeff << offset) >> (8 * k) & (0xff << (k == 0 ? offset : 0));
            }

            // FIXME: should be able to do it without this test
            if (i != polys_count - 1 || j != _N_ - 1) {
                data[index] &= (0xff >> (8 - (k == 0 ? offset : 0)));
                data[index] |= (coeff << offset) >> (8 * k) & (0xff << (k == 0 ? offset : 0));
            }

            offset = (offset + valid_bits) & 0x07;
        }
    }
}

void poly_unpack(uint8_t valid_bits, const uint8_t *data, size_t polys_count, uint8_t is_signed, poly_t *poly) {
    uint8_t offset = 0, next_offset;
    size_t index = 0;

    for (size_t i = 0; i < polys_count; i++) {
        for (size_t j = 0; j < _N_; j++) {
            size_t k;

            poly[i].coeffs[j] = 0;

            for (k = 0; k < (valid_bits + offset) / 8; k++)
                poly[i].coeffs[j] |= (int32_t) data[index++] << (8 * k);

            next_offset = (offset + valid_bits) & 0x07;

            poly[i].coeffs[j] |= ((int32_t) data[index] & ((0xff >> (8 - next_offset)))) << (8 * k);
            poly[i].coeffs[j] >>= offset;

            if (is_signed)
                poly[i].coeffs[j] |= ((poly[i].coeffs[j] & (0x01 << (valid_bits - 1))) << (32 - valid_bits)) >> (32 - valid_bits);

            offset = next_offset;
        }
    }
}


void poly_gen_commit(const uint8_t ck_seed[SEED_BYTES], const uint8_t r_seed[SEED_BYTES], poly_t f[][_K_])
{
    int8_t bound_exceeded = 0;

    poly_t r_kj = {0};
    poly_t ck_ik = {0};
    uint32_t nonce = 0;

    memset(f, 0, _K_*_K_*_N_);

    for(size_t k = 0; k < TC_COLS; k++) {
    	nonce = 0;
    	for (size_t j = 0; j < _K_; j++) {
			//generate r[j][k]
    		do {
    			nonce++;
    			sample_normal_from_seed(r_seed, j*TC_COLS+k+nonce, 0, TC_S, _N_, r_kj.coeffs);
    		} while(!poly_check_norm(&r_kj, 1, TC_B));

    		for(size_t i = 0; i < _K_; i++) {

        		//generate ck[i][k]
        		poly_uniform(ck_seed, 1, i*TC_COLS+k, (poly_t*) &ck_ik);

        		// f[i][j] += r[j][k] + ck[i][k]
    			for(size_t n = 0; n < _N_; n++)
    			    f[i][j].coeffs[n] += montgomery_reduce((int64_t) ck_ik.coeffs[n] * r_kj.coeffs[n]);
    		}
    	}
    }
    poly_reduce((poly_t*)f, _K_*_K_);
    poly_invntt_tomont((poly_t*)f, _K_*_K_);
}
