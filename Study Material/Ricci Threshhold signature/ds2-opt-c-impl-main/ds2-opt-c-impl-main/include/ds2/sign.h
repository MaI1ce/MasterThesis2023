#ifndef DS2_SIGN_H
#define DS2_SIGN_H

#include <stdlib.h>
#include <time.h>

#include "ds2/commit.h"
#include "ds2/party.h"
#include "ds2/poly.h"

extern clock_t sign_clock;

typedef struct {
    poly_t z1[L];
    poly_t z2[K];
    uint8_t c[SEED_BYTES];
    poly_t r[K][TC_COLS];
} sign_t;

int8_t sign(const party_t *party, const uint8_t *msg, size_t msg_len, sign_t *sig);

#endif