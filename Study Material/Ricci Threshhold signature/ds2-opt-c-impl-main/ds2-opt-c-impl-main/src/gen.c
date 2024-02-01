#include <stdlib.h>
#include <stdio.h>

#include "ds2/benchmark.h"
#include "ds2/commit.h"
#include "ds2/fips202.h"
#include "ds2/gen.h"
#include "ds2/party.h"
#include "ds2/poly.h"
#include "ds2/rand.h"
#include "ds2/util.h"

// Generates matrix A, which is a sum of random matrices sampled by each party.
// If this function executes succesfully, matrix A will be stored in `party->pk.A`.
// Returns 0 on success.
int8_t generate_matrix(party_t *party) {
    TB_START(EXPAND_A_INDEX)

    uint8_t seed[N_PARTIES][SEED_BYTES];
    uint8_t g[N_PARTIES][L1];

    #ifdef USE_SAFE_RANDOM
    arc4random_buf(seed[party->n], SEED_BYTES);
    #else
    for (size_t i = 0; i < SEED_BYTES; i++)
        seed[party->n][i] = rand();
    #endif

    h1(seed[party->n], party->n, g[party->n]);

    // Send out commitment to seed part for matrix `A` expansion
    broadcast(party, g[party->n], L1);

    // Receive commitments to seed part for matrix `A` expansion from other participants
    receive_all(party, g[0], L1);

    // Send out seed for matrix `A` expansion
    broadcast(party, seed[party->n], SEED_BYTES);

    // Receive seed for matrix `A` expansion from other participants and check commitments from each participants.
    // When any commitment is incorrect ABORT.
    for (size_t i = 0; i < N_PARTIES; i++) {
        if (i != party->n) {
            receive(party, i, SEED_BYTES, seed[i]);

            h1(seed[i], i, g[party->n]);

            if (cmp(g[party->n], g[i], L1) != 1) {
                send_abort(party, 1);
                check_abort(party);

                return -1;
            }
        }
    }

    // Send success and check if any party aborted.
    send_abort(party, 0);
    if (check_abort(party) != 0) return -1;

    keccak_state_t state;

    keccak_init(&state);
    shake128_absorb(&state, &seed[0][0], N_PARTIES * SEED_BYTES);
    shake128_finalize(&state);
    shake128_squeeze(&state, SEED_BYTES, party->rho);

    poly_uniform(party->rho, K * L, &party->A[0][0]);

    TB_END(EXPAND_A_INDEX)

    return 0;
}

// Generates private key sk and public key pk = (A, t).
// Returns 0 on success.
int8_t generate_key_pair(party_t *party) {
    if (generate_matrix(party) != 0)
        return -1;

    TB_START(KEYPAIR_INDEX)

    uint8_t seed[SEED_BYTES];
    poly_t s1[L];
    uint8_t t1_packed[K * POLY_T1_PACKED_BYTES] = {};
    uint8_t g[N_PARTIES][L2];

    poly_t *t1 = party->t1[party->n];

    #ifdef USE_SAFE_RANDOM
    arc4random_buf(seed, SEED_BYTES);
    #else
    for (size_t i = 0; i < SEED_BYTES; i++)
        seed[i] = rand();
    #endif

    poly_eta(seed, 0, L, party->s1);
    poly_eta(seed, L, K, party->s2);

    poly_copy(party->s1, L, s1);

    // Compute t_n = (A | I) * s_n
    poly_ntt(s1, L);
    poly_product(party->A, s1, t1);
    poly_reduce(t1, K);
    poly_invntt_tomont(t1, K);

    poly_add(t1, party->s2, K, t1);
    poly_addq(t1, K);
    poly_power2round(t1, K, party->t0);

    poly_pack(T1_BITS, t1, K, t1_packed);

    h2(t1_packed, party->n, g[party->n]);

    // Send out commitment to `t`
    broadcast(party, g[party->n], L2);

    // Receive commitments to `t` from other participants
    receive_all(party, (uint8_t*) g, L2);

    // Send out `t`
    broadcast(party, t1_packed, sizeof(t1_packed));

    // Receive `t` from other participants and check commitments from each participants.
    // When any commitment is incorrect ABORT.
    for (size_t i = 0; i < N_PARTIES; i++) {
        if (i != party->n) {
            receive(party, i, sizeof(t1_packed), t1_packed);

            h2(t1_packed, i, g[party->n]);

            if (cmp(g[party->n], g[i], L2) != 1) {
                send_abort(party, 1);
                check_abort(party);

                printf("ABORTING T GENERATION\n");

                return -1;
            }

            poly_unpack(T1_BITS, t1_packed, K, 0, party->t1[i]);

            poly_add(t1, party->t1[i], K, t1);
        }
    }

    // Send success and check if any party aborted.
    send_abort(party, 0);
    if (check_abort(party) != 0) return -1;

    poly_freeze(t1, K);

    keccak_state_t state;

    keccak_init(&state);
    shake128_absorb(&state, party->rho, SEED_BYTES);
    shake128_absorb(&state, (uint8_t*) t1, K * sizeof(poly_t));
    shake128_finalize(&state);
    shake128_squeeze(&state, SEED_BYTES, party->tr);

    TB_END(KEYPAIR_INDEX)

    return 0;
}