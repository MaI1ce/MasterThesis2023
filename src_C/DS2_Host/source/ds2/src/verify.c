#include "../../ds2_stm/include/ds2/verify.h"

#include <stdio.h>

#include "../../ds2_stm/include/ds2/fips202.h"
#include "../../ds2_stm/include/ds2/util.h"

// Returns 1 when signature is valid, 0 otherwise.
uint8_t verify(const party_t *party, const uint8_t *msg, size_t msg_len, const sign_t *sig) {
    poly_t A[K][L];
    poly_t t1[K];
    poly_t z1[L];
    poly_t c = {};
    poly_t w1[K];

    if (!poly_check_norm(sig->z1, L, N_PARTIES_SQRT * _B_) || !poly_check_norm(sig->z2, K, N_PARTIES_SQRT * _B_))
        return 0;

    poly_uniform(party->rho, K * L, &A[0][0]);

    poly_copy(sig->z1, L, z1);
    poly_challenge(sig->c, &c);

    poly_const_mul(party->t1[party->n], 1 << D, K, t1);

    poly_ntt(z1, K);
    poly_ntt(t1, K);
    poly_ntt(&c, 1);

    poly_product(A, z1, w1);
    poly_mul_pointwise(t1, &c, K, t1);

    poly_reduce(w1, K);
    poly_reduce(t1, K);

    poly_invntt_tomont(w1, K);
    poly_invntt_tomont(t1, K);

    poly_sub(w1, t1, K, w1);
    poly_add(w1, sig->z2, K, w1);

    poly_freeze(w1, K);

    poly_t ck[2][TC_COLS];
    poly_t f[K][2];
    uint8_t f_packed[2 * 3 * K * N];
    uint8_t tr[SEED_BYTES];
    uint8_t challenge_seed[SEED_BYTES];

    keccak_state_t state;


    keccak_init(&state);
    shake128_absorb(&state, party->rho, SEED_BYTES);
    shake128_absorb(&state, (uint8_t*) party->t1[party->n], K * sizeof(poly_t));
    shake128_finalize(&state);
    shake128_squeeze(&state, SEED_BYTES, tr);

    h3(msg, msg_len, tr, ck);

    if (poly_commit2(w1, K, ck, sig->r, f) != 0) return 0;

    poly_freeze((poly_t*) f, 2 * K);

    poly_pack(24, (poly_t*) f, 2 * K, f_packed);

    keccak_init(&state);

    shake256_absorb(&state, f_packed, sizeof(f_packed));
    shake256_absorb(&state, msg, msg_len);
    shake128_absorb(&state, tr, SEED_BYTES);
    shake128_finalize(&state);
    shake128_squeeze(&state, SEED_BYTES, challenge_seed);

    return cmp(sig->c, challenge_seed, SEED_BYTES);
}