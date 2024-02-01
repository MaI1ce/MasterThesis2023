#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>

#include "ds2/benchmark.h"
#include "ds2/gen.h"
#include "ds2/party.h"
#include "ds2/sign.h"
#include "ds2/verify.h"
#include "ds2/socket.h"

int main() {
    party_t party;
    sign_t sig;

    party.n = 0;

    FILE *null_file = fopen("/dev/null", "wb");

    clock_t generate_key_pair_clock = clock();
    clock_t sign_clock = clock();
    clock_t verify_clock = clock();

    for (size_t i = 0; i < 1; i++) {
        srand(54321 + i);
        printf("%d\n", rand());
        srand(54321 + i);

        FILE *files[N_PARTIES];

        for (size_t j = 1; j < N_PARTIES; j++) {
            char filename[50];
            sprintf(filename, "%d/%zu_%zu.bin", N_PARTIES, j, i);

            files[j] = fopen(filename, "rb");

            party.rx_fds[j] = fileno(files[j]);
            party.tx_fds[j] = fileno(null_file);
        }

        printf("0: rx[%d, %d, %d], tx[%d, %d, %d]\n", party.rx_fds[0], party.rx_fds[1], party.rx_fds[2], party.tx_fds[0], party.tx_fds[1], party.tx_fds[2]);

        clock_t previous = read_write_clocks;
        clock_t start = clock();
        int result = generate_key_pair(&party);
        generate_key_pair_clock += clock() - start - (read_write_clocks - previous);

        printf("gen_key: %d\n", result);

        previous = read_write_clocks;
        start = clock();
        result = sign(&party, "Test", 5, &sig);
        sign_clock += clock() - start - (read_write_clocks - previous);
        printf("sign: %d\n", result);

        start = clock();
        result = verify(&party, "Test", 5, &sig);
        verify_clock += clock() - start;
        printf("verify: %d\n", result);

        // long x = 0;
        // for (size_t i = 0; i < K; i++)
        //     for (size_t j = 0; j < L; j++)
        //         for (size_t k = 0; k < N; k++)
        //             x += party.A[i][j].coeffs[k];

        // printf("A: %lx\n", x);

        // for (size_t i = 0; i < K; i++)
        //     for (size_t k = 0; k < N; k++)
        //         x += party.t1[party.n][i].coeffs[k];

        // printf("t1: %lx\n", x);

        for (size_t i = 1; i < N_PARTIES; i++)
            fclose(files[i]);
    }

    printf("gen_key: %f\n", (generate_key_pair_clock / (double) CLOCKS_PER_SEC) / 200.0 * 1000.0);
    printf("sign: %f\n", (sign_clock / (double) CLOCKS_PER_SEC) / 200.0 * 1000.0);
    printf("verify: %f\n", (verify_clock / (double) CLOCKS_PER_SEC) / 200.0 * 1000.0);

    fclose(null_file);

    print_results();

    return 0;
}