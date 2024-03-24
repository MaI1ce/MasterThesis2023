#ifndef DS2_POLY_H
#define DS2_POLY_H

#include <stdlib.h>
#include <stdint.h>
#include "params.h"

typedef struct {
    int32_t coeffs[_N];
} poly_t;

void poly_add(const poly_t *a, const poly_t *b, size_t polys_count, poly_t *c);

void poly_sub(const poly_t *a, const poly_t *b, size_t polys_count, poly_t *c);

void poly_mul_pointwise(const poly_t *a, const poly_t *b, size_t polys_count, poly_t *c);

void poly_const_mul(const poly_t *a, int32_t constant, size_t polys_count, poly_t *c);

void poly_addq(poly_t *poly, size_t polys_count);

void poly_reduce(poly_t *poly, size_t polys_count);

void poly_freeze(poly_t *poly, size_t polys_count);

void poly_center(poly_t *poly, size_t polys_count);

void poly_ntt(poly_t *poly, size_t polys_count);

void poly_invntt_tomont(poly_t *poly, size_t polys_count);

void poly_product(const poly_t a[K][L], const poly_t b[L], poly_t c[K]);

void poly_product2(const poly_t a[2][TC_COLS], const poly_t b[TC_COLS], poly_t c[2]);

void poly_copy(const poly_t *poly, size_t polys_count, poly_t *copy);

void poly_uniform(const uint8_t seed[SEED_BYTES], size_t polys_count, size_t nonce, poly_t *poly);

void poly_eta(const uint8_t seed[SEED_BYTES], uint32_t nonce, size_t polys_count, poly_t *poly);

void poly_normal(const uint8_t seed[SEED_BYTES], uint32_t nonce, uint32_t stddev, size_t polys_count, poly_t *poly);

void poly_challenge(const uint8_t seed[SEED_BYTES], poly_t *poly);

void poly_make_hint(const poly_t *w1, size_t polys_count, poly_t *h);

void poly_use_hint(const poly_t *h, size_t polys_count, poly_t *w1);

void poly_commit(const poly_t *poly, size_t polys_count, const poly_t ck[][TC_COLS], poly_t r[][TC_COLS], poly_t f[][2]);

int8_t poly_commit2(const poly_t *poly, size_t polys_count, const poly_t ck[][TC_COLS], const poly_t r[][TC_COLS], poly_t f[][2]);

uint8_t poly_open(const poly_t *poly, size_t polys_count, const poly_t ck[][TC_COLS], const poly_t r[][TC_COLS], const poly_t f[][2]);

void poly_power2round(poly_t *r1, size_t polys_count, poly_t *r0);

void poly_decompose(poly_t *r1, size_t polys_count, poly_t *r0);

uint8_t poly_reject(const poly_t z1[L], const poly_t z2[K], const poly_t cs1[L], const poly_t cs2[K]);

uint8_t poly_check_norm(const poly_t *poly, size_t polys_count, double bound);

void poly_pack(uint8_t valid_bits, const poly_t *poly, size_t polys_count, uint8_t *data);

void poly_unpack(uint8_t valid_bits, const uint8_t *data, size_t polys_count, uint8_t is_signed, poly_t *poly);

void poly_gen_commit(const uint8_t ck_seed[SEED_BYTES], const uint8_t r_seed[SEED_BYTES], poly_t f[][K]);

#endif
