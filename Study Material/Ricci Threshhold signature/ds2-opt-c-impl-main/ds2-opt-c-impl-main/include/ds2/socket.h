#ifndef SOCKET_H
#define SOCKET_H

#include "ds2/params.h"

int initialize_server(int id, int client_fds[N_PARTIES]);

int initialize_client(const char* address, int server_id, int id);

#endif