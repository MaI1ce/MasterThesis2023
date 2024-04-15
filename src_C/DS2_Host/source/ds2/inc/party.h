#ifndef DS2_PARTY_H
#define DS2_PARTY_H

#include <stdlib.h>
#include <time.h>

#include "poly.h"

extern clock_t read_write_clocks;

typedef struct {
    uint32_t n;
    uint32_t iteration;

    int32_t rx_fds[N_PARTIES];
    int32_t tx_fds[N_PARTIES];

    // pk
    uint8_t rho[SEED_BYTES];
    poly_t A[_K_][_L_];
    poly_t t1[N_PARTIES][_K_];

    // sk
    uint8_t tr[SEED_BYTES];
    poly_t s1[_L_];
    poly_t s2[_K_];
    poly_t t0[_K_];
} party_t;

void send_data(const party_t *party, uint32_t n, const uint8_t *data, size_t data_len);

void broadcast(const party_t *party, const uint8_t *data, size_t data_len);

void receive(const party_t *party, uint32_t n, size_t data_len, uint8_t *data);

void receive_all(const party_t *party, uint8_t *data, size_t data_len);

uint8_t check_abort(const party_t *party);

void send_abort(const party_t *party, uint8_t abort);

#endif
