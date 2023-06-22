#ifndef SERVER_UTILS_H
#define SERVER_UTILS_H

#include <stdbool.h>

#define SETUP_SERVER_SOCKET_ERROR "setupServerSocket()"
#define ACCEPT_CONNECTION_ERROR   "acceptConnection()"
#define MAX_QUEUED_CONNECTIONS 20

int setupIpv4ServerSocket(int);
int setupIpv6ServerSocket(int);
int setupServerSocket(int);
int setup_udp_ipv4_socket(int port);
int acceptConnection(int serverSock);
int handleConnection(int clientSocket);

#endif