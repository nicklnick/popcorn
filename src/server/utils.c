#include "utils.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "wrapper-functions.h"

#define BUFSIZE 256

int setupServerSocket(int port) {
    // IPv4 address
    // INADDR_ANY  (0.0.0.0)  means any address for binding
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    // TODO: Accept both ipv4 and ipv6 connections

    int server = _socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    _setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    _bind(server, (struct sockaddr *)&addr, sizeof(addr));
    _listen(server, 20);

    return 0;
}

int acceptConnection(int serverSock) {
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    return _accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
}

int handleConnection(int clientSocket) {
    char buffer[BUFSIZE] = {0};

    ssize_t bytesRcvd = _recv(clientSocket, buffer, BUFSIZE, 0);
    while (bytesRcvd > 0) {
        ssize_t bytesSent = _send(clientSocket, buffer, bytesRcvd, 0);
        if (bytesSent != bytesRcvd) {
            perror("send()");
            exit(EXIT_FAILURE);
        }

        bytesRcvd = _recv(clientSocket, buffer, BUFSIZE, 0);
    }

    close(clientSocket);
    return 0;
}