#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>

#include "ds2/benchmark.h"
#include "ds2/gen.h"
#include "ds2/party.h"
#include "ds2/sign.h"
#include "ds2/verify.h"
#include "ds2/socket.h"


void run(int n, int seed, size_t iteration) {
    printf("running: %d, %d, %d\n", n, seed, iteration);
    party_t party;
    sign_t sig;

    party.n = n;
    party.iteration = iteration;
    srand(seed);

    if (n != 0) {
        char filename[50];
        sprintf(filename, "%d/%d_%d.bin", N_PARTIES, party.n, party.iteration);
        FILE *file = fopen(filename, "wb");

        fclose(file);
    }

    int client_fds[N_PARTIES];

    int server_fd = initialize_server(party.n, client_fds);

    for (size_t i = 0; i < N_PARTIES; i++) {
        if (i < party.n) {
            int fd;

            // if (i == 0)
            //     fd = initialize_client("169.254.96.102", i, party.n);
            // else
                fd = initialize_client("127.0.0.1", i, party.n);

            if (fd == -1) return;

            party.tx_fds[i] = fd;
            party.rx_fds[i] = fd;
        } else {
            if (client_fds[i] == -1) return;

            party.tx_fds[i] = client_fds[i];
            party.rx_fds[i] = client_fds[i];
        }
    }

    usleep(10000 * (n + 1));

    printf("gen_key: %d\n", generate_key_pair(&party));
    // long x = 0;
    // for (size_t i = 0; i < K; i++)
    //     for (size_t j = 0; j < N; j++)
    //         x += party.t1[party.n][i].coeffs[j];

    // printf("rho: %x %x\n", party.rho[0], party.rho[1]);
    // printf("X: %x\n", x);

    printf("sign: %d\n", sign(&party, "Test", 5, &sig));
    printf("verify: %d\n", verify(&party, "Test", 5, &sig));

    close(server_fd);
    for (size_t i = 0; i < N_PARTIES; i++) {
        close(party.tx_fds[i]);
        close(party.rx_fds[i]);
    }

    usleep(100000 * n);
}

int main(int argc, char** argv) {
    for (size_t i = 0; i < 20; i++)
        run(atoi(argv[1]), atoi(argv[2]) + i, i);

    print_results();

    return 0;
}