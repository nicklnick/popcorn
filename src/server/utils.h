#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stdbool.h>

#define SETUP_SERVER_SOCKET_ERROR "setupServerSocket()"
#define ACCEPT_CONNECTION_ERROR   "acceptConnection()"

int setupServerSocket(int);
int setup_udp_ipv4_socket(int port);
int acceptConnection(int serverSock);
int handleConnection(int clientSocket);

#endif