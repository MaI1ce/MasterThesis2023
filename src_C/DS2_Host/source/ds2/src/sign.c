#include "../../ds2_stm/include/ds2/sign.h"

#include <stdio.h>
#include <stdlib.h>

#include "../../ds2_stm/include/ds2/benchmark.h"
#include "../../ds2_stm/include/ds2/commit.h"
#include "../../ds2_stm/include/ds2/fips202.h"
#include "../../ds2_stm/include/ds2/poly.h"
#include "../../ds2_stm/include/ds2/rand.h"
#include "../../ds2_stm/include/ds2/reduce.h"

// Returns 0 on success.
int8_t sign(const party_t *party, const uint8_t *msg, size_t msg_len, sign_t *sig) {
    uint8_t seed[SEED_BYTES];
    uint32_t nonce = 0;
    poly_t ck[2][TC_COLS];
    poly_t y1[L], y2[K], y1_hat[L];
    poly_t w[K];
    poly_t f[N_PARTIES][K][2];
    poly_t f_sum[K][2];
    uint8_t f_packed[2 * 3 * K * N];
    keccak_state_t state;
    poly_t c = {};
    poly_t cs1[L], cs2[K];
    poly_t s1[L], s2[K];
    poly_t t0[K], t1[K];
    uint8_t z1_packed[3 * L * N], z2_packed[3 * K * N];
    poly_t z1_received[L], z2_received[K];
    poly_t r_received[TC_COLS];

    //NO MALLOC IN STM !!!
    uint8_t *r_packed = malloc(sizeof(uint8_t) * K * 3 * TC_COLS * N);
    uint8_t *r_packed_received = malloc(sizeof(uint8_t) * 3 * TC_COLS * N);
    uint8_t rej;

    #ifdef USE_SAFE_RANDOM
    arc4random_buf(seed, SEED_BYTES);
    #else
    for (size_t i = 0; i < SEED_BYTES; i++)
        seed[i] = rand();
    #endif

    // Compute per-message commitment key.
    h3(msg, msg_len, party->tr, ck);

    int iterations = 0;

    do {
        TB_START(SIGN_ITERATION_INDEX)

        poly_normal(seed, nonce, SIGMA, L, y1);
        nonce += L;
        poly_normal(seed, nonce, SIGMA, K, y2);
        nonce += K;

        // Compute w_n = (A | I) * y_n
        poly_copy(y1, L, y1_hat);
        poly_ntt(y1_hat, L);
        poly_product(party->A, y1_hat, w);
        poly_reduce(w, K);
        poly_invntt_tomont(w, K);

        poly_add(w, y2, K, w);
        poly_freeze(w, K);

        poly_commit(w, K, ck, sig->r, f[party->n]);

        poly_freeze((poly_t*) f[party->n], 2 * K);

        poly_pack(23, (poly_t*) f[party->n], 2 * K, f_packed);

        poly_copy((poly_t*) f[party->n], 2 * K, (poly_t*) f_sum);

        broadcast(party, f_packed, sizeof(f_packed));

        for (size_t i = 0; i < N_PARTIES; i++) {
            if (i != party->n) {
                receive(party, i, sizeof(f_packed), f_packed);

                poly_unpack(23, f_packed, 2 * K, 0, (poly_t*) f[i]);

                poly_add((poly_t*) f_sum, (poly_t*) f[i], 2 * K, (poly_t*) f_sum);
            }
        }

        poly_freeze((poly_t*) f_sum, 2 * K);

        poly_pack(24, (poly_t*) f_sum, 2 * K, f_packed);

        // H(com, msg, pk)
        keccak_init(&state);

        shake256_absorb(&state, f_packed, sizeof(f_packed));
        shake256_absorb(&state, msg, msg_len);
        shake128_absorb(&state, party->tr, SEED_BYTES);
        shake128_finalize(&state);
        shake128_squeeze(&state, SEED_BYTES, sig->c);

        poly_challenge(sig->c, &c);

        // z_n = c * s_n + y_n
        poly_copy(party->s1, L, s1);
        poly_copy(party->s2, K, s2);

        poly_ntt(s1, L);
        poly_ntt(s2, K);
        poly_ntt(&c, 1);

        poly_mul_pointwise(s1, &c, L, cs1);
        poly_mul_pointwise(s2, &c, K, cs2);

        poly_reduce(cs1, L);
        poly_reduce(cs2, K);

        poly_invntt_tomont(cs1, L);
        poly_invntt_tomont(cs2, K);

        poly_add(cs1, y1, L, sig->z1);
        poly_add(cs2, y2, K, sig->z2);

        rej = poly_reject(sig->z1, sig->z2, cs1, cs2);

        send_abort(party, rej);

        rej |= check_abort(party);

        if (!rej) {
            poly_copy(party->t0, K, t0);

            poly_ntt(t0, K);
            poly_mul_pointwise(t0, &c, K, t0);
            poly_reduce(t0, K);
            poly_invntt_tomont(t0, K);

            poly_sub(sig->z2, t0, K, sig->z2);

            poly_center(sig->z1, L);
            poly_center(sig->z2, K);

            rej = !poly_check_norm(sig->z1, L, _B) || !poly_check_norm(sig->z2, K, _B);

            send_abort(party, rej);

            rej |= check_abort(party);
        }

        if (rej)
            for (size_t i = 0; i < N; i++)
                c.coeffs[i] = 0;

        iterations++;

        TB_END(SIGN_ITERATION_INDEX)
    } while (rej == 1);

    printf("ITERATIONS: %d\n", iterations);

    poly_addq(sig->z1, L);
    poly_addq(sig->z2, K);

    poly_pack(23, sig->z1, L, z1_packed);
    poly_pack(23, sig->z2, K, z2_packed);

    broadcast(party, z1_packed, sizeof(z1_packed));
    broadcast(party, z2_packed, sizeof(z2_packed));

    for (size_t i = 0; i < K; i++)
        poly_pack(23, sig->r[i], TC_COLS, &r_packed[i * TC_COLS * 3 * N]);

    for (size_t i = 0; i < N_PARTIES; i++) {
        if (i != party->n) {
            receive(party, i, sizeof(z1_packed), z1_packed);
            receive(party, i, sizeof(z2_packed), z2_packed);

            poly_unpack(23, z1_packed, L, 0, z1_received);
            poly_unpack(23, z2_packed, K, 0, z2_received);

            poly_center(z1_received, L);
            poly_center(z2_received, K);

            if (!poly_check_norm(z1_received, L, _B) || !poly_check_norm(z2_received, K, _B)) {
                printf("COULD NOT CHECK NORM\n");
                send_abort(party, 1);
                check_abort(party);

                return -1;
            }

            poly_add(sig->z1, z1_received, L, sig->z1);
            poly_add(sig->z2, z2_received, K, sig->z2);

            // w = Az1 - ct1 * 2^D
            poly_const_mul(party->t1[i], 1 << D, K, t1);

            poly_ntt(z1_received, L);
            poly_ntt(t1, K);

            poly_product(party->A, z1_received, w);
            poly_mul_pointwise(t1, &c, K, t1);

            poly_reduce(w, K);
            poly_reduce(t1, K);

            poly_invntt_tomont(w, K);
            poly_invntt_tomont(t1, K);

            poly_sub(w, t1, K, w);
            poly_add(w, z2_received, K, w);

            poly_freeze(w, K);

            for (size_t j = 0; j < K; j++) {
                send_data(party, i, &r_packed[j * TC_COLS * 3 * N], TC_COLS * 3 * N);
                for (size_t k = 0; k < TC_COLS; k++)
                    receive(party, i, 3 * N, &r_packed_received[k * 3 * N]);
                poly_unpack(23, r_packed_received, TC_COLS, 1, r_received);

                poly_add(sig->r[j], r_received, TC_COLS, sig->r[j]);

                if (!poly_open(&w[j], 1, ck, r_received, &f[i][j])) {
                    printf("COULD NOT OPEN: %zu, %zu\n", i, j);
                    send_abort(party, 1);
                    check_abort(party);

                    return -1;
                }
            }
        }
    }

    poly_center(sig->z1, L);
    poly_center(sig->z2, K);

    // Send success and check if any party aborted.
    send_abort(party, 0);
    if (check_abort(party) != 0) return -1;

    free(r_packed);
    free(r_packed_received);

    return 0;
}