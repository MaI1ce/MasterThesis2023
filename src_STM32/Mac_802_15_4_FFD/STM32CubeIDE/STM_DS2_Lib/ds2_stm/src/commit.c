#include "commit.h"

#include <stdio.h>
#include <math.h>

#include "fips202.h"
#include "poly.h"
#include "rand.h"
#include "reduce.h"

void h0(const int32_t com[K][_N][2], const uint8_t *msg, size_t msg_len, const uint8_t tr[SEED_BYTES], uint8_t c[SEED_BYTES]) {
    keccak_state_t state;

    keccak_init(&state);
    // FIXME: will be incorrect becouse of endiannes
    shake256_absorb(&state, (uint8_t*) com, K * _N * 2 * sizeof(int32_t));
    shake256_absorb(&state, msg, msg_len);
    shake256_absorb(&state, tr, SEED_BYTES);
    shake256_finalize(&state);
    shake256_squeeze(&state, SEED_BYTES, c);
}

void h1(const uint8_t seed[SEED_BYTES], uint32_t n, uint8_t g[L1]) {
    keccak_state_t state;

    keccak_init(&state);
    shake256_absorb(&state, seed, SEED_BYTES);
    shake256_absorb_nonce(&state, n);
    shake256_finalize(&state);
    shake256_squeeze(&state, L1, g);
}

void h2(const uint8_t t[K * POLY_T1_PACKED_BYTES], uint32_t n, uint8_t g[L2]) {
    keccak_state_t state;

    keccak_init(&state);
    shake256_absorb(&state, t, K * POLY_T1_PACKED_BYTES);
    shake256_absorb_nonce(&state, n);
    shake256_finalize(&state);
    shake256_squeeze(&state, L2, g);
}

void h3(const uint8_t *msg, size_t msg_len, const uint8_t tr[SEED_BYTES], poly_t ck[2][TC_COLS]) {
    keccak_state_t state;

    uint8_t seed[SEED_BYTES];

    keccak_init(&state);
    shake256_absorb(&state, msg, msg_len);
    shake256_absorb(&state, tr, SEED_BYTES);
    shake256_finalize(&state);
    shake256_squeeze(&state, SEED_BYTES, seed);

    poly_uniform(seed, 2 * TC_COLS, 0, (poly_t*) ck);
}

// Returns 0 on success
int8_t commit(const poly_t *x, const poly_t A[2][TC_COLS], const poly_t r[TC_COLS], double bound, poly_t f[2]) {
    if (!poly_check_norm(r, TC_COLS, bound)) return -1;

    poly_product2(A, r, f);
    poly_reduce(f, 2);
    poly_invntt_tomont(f, 2);

    poly_add(&f[1], x, 1, &f[1]);

    return 0;
}
