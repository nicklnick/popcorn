#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

int setupServerSocket(int port) {

    // IPv4 address
    // INADDR_ANY  (0.0.0.0)  means any address for binding
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    //TODO: Accept both ipv4 and ipv6 connections

    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(server < 0){
        perror("socket()");
        goto finally;
    }

    setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int));

    if(bind(server,(struct sockaddr *)&addr, sizeof(addr)) < 0){
        perror("bind()");
        goto finally;
    }

    if (listen(server, 20) < 0) {
        perror("listen()");
        goto finally;
    }

    int ret = 0;
    return ret;

    finally:
    ret = -1;
    return ret;
}

int acceptConnection(int serverSock){

    struct sockaddr_storage clientAddr;
    socklen_t  clientAddrLen = sizeof(clientAddr);

    int clientSocket = accept(serverSock,(struct sockaddr *) &clientAddr, &clientAddrLen);
    if(clientSocket < 0){
        perror("accept()");
        return -1;
    }

    return clientSocket;
}