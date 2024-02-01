#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ds2/socket.h"

int initialize_server(int id, int client_fds[N_PARTIES]) {
    int server_fd;

    if ((server_fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return -1;

    int opt = 1;

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)))
        return -1;

    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(9000 + id);

    int addrlen = sizeof(addr);

    if (bind(server_fd, (struct sockaddr*) &addr, addrlen) < 0)
        return -1;

    if (listen(server_fd, N_PARTIES) < 0)
        return -1;

    for (size_t i = 0; i < N_PARTIES - id - 1; i++) {
        int client_fd;

        if ((client_fd = accept(server_fd, (struct sockaddr*) &addr, (socklen_t*) &addrlen)) < 0)
            return -1;

        char buffer[5] = {};
        int size = read(client_fd, buffer, 5);

        printf("read: %d %d %s\n", client_fd, size, buffer);

        int client_id = atoi(buffer);

        if (client_id >= 0 && client_id < N_PARTIES)
            client_fds[client_id] = client_fd;
    }

    return server_fd;
}

int initialize_client(const char* address, int server_id, int id) {
    int fd;

    if ((fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
        return -1;

    struct sockaddr_in addr;
    addr.sin_family = PF_INET;
    addr.sin_port = htons(9000 + server_id);

    if (inet_pton(PF_INET, address, &addr.sin_addr) <= 0)
        return -1;

    printf("Connecting to address %s:%d\n", address, 9000 + server_id);

    int status, reconnects = 0;

    while ((status = connect(fd, (struct sockaddr*) &addr, sizeof(addr))) < 0 && reconnects < 10) {
        reconnects++;
        usleep(500);
        printf("reconnecting\n");

        close(fd);
        if ((fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
            return -1;
    }

    if (status != 0) {
        printf("%s\n", strerror(errno));
        return -1;
    }

    char initial_msg[5] = { 0, 0, 0, 0, 0};
    sprintf(initial_msg, "%d", id);

    int size = write(fd, initial_msg, 5);

    printf("send: %d %d %s\n", fd, size,  initial_msg);

    return fd;
}
