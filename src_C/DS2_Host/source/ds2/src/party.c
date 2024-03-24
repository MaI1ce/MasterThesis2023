#include "party.h"

#include <stdio.h>
#include <time.h>
#include <unistd.h>


clock_t read_write_clocks;

// TODO: handle errors

void send_data(const party_t *party, uint32_t n, const uint8_t *data, size_t data_len) {
    int w = write(party->tx_fds[n], data, data_len);

    if (w != data_len)
        printf("ERROR WRITING: %d, %d\n", w, data_len);

    // printf("WRITE: \n");
    // for (size_t i = 0; i < w; i++)
    //     printf("%hhx", data[i]);
    // printf("\n");

    // if (n == 0) {
    //     char filename[50];
    //     sprintf(filename, "%d/%d_%d.bin", N_PARTIES, party->n, party->iteration);
    //     FILE *file = fopen(filename, "ab");

    //     fwrite(data, sizeof(uint8_t), data_len, file);

    //     fclose(file);
    // }
}

void broadcast(const party_t *party, const uint8_t *data, size_t data_len) {
    for (size_t i = 0; i < N_PARTIES; i++)
        if (i != party->n)
            send_data(party, i, data, data_len);
}

void receive(const party_t *party, uint32_t n, size_t data_len, uint8_t *data) {
    clock_t start = clock();
    int r = read(party->rx_fds[n], data, data_len);
    read_write_clocks += clock() - start;

    while (r != data_len) {
        r += read(party->rx_fds[n], data + r, data_len - r);
    }

    // printf("ERROR READING: %d, %d\n", r, data_len);
    // for (size_t i = 0; i < r; i++)
    //     printf("%hhx", data[i]);
}

void receive_all(const party_t *party, uint8_t *data, size_t data_len) {
    for (size_t i = 0; i < N_PARTIES; i++)
        if (i != party->n)
            receive(party, i, data_len, data + (data_len * i));
}

void send_abort(const party_t *party, uint8_t abort) {
    broadcast(party, &abort, 1);
}

// Returns 1 on abort.
uint8_t check_abort(const party_t *party) {
    uint8_t abort = 0, received_abort = 0;

    for (size_t i = 0; i < N_PARTIES; i++) {
        if (i != party->n)
            receive(party, i, 1, &received_abort);
        abort |= received_abort;
    }

    return abort;
}
