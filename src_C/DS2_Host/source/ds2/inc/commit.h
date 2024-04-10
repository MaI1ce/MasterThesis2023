#ifndef DS2_COMMIT_H
#define DS2_COMMIT_H

#include <stdlib.h>

#include "params.h"
#include "party.h"
#include "poly.h"

#ifdef __cplusplus
#include <cmath>
extern "C" {
#endif

	void h0(const int32_t com[_K][_N_][2], const uint8_t* msg, size_t msg_len, const uint8_t tr[SEED_BYTES], uint8_t c[SEED_BYTES]);

	void h1(const uint8_t seed[SEED_BYTES], uint32_t n, uint8_t g[L1]);

	void h2(const uint8_t t[_K * POLY_T1_PACKED_BYTES], uint32_t n, uint8_t g[L2]);

	void h3(const uint8_t* msg, size_t msg_len, const uint8_t tr[SEED_BYTES], poly_t ck[2][TC_COLS]);

	int8_t commit(const poly_t* x, const poly_t A[2][TC_COLS], const poly_t r[TC_COLS], double bound, poly_t f[2]);

#ifdef __cplusplus
}
#endif
#endif
