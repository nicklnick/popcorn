#include "utils.h"
#include "../parser/command_parser.h"
#include "../session/session.h"
#include "../sm/sm.h"
#include "wrapper-functions.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

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
    session_ptr client_session = new_client_session(clientSocket);

    while (get_session_state(client_session) == AUTHORIZATION) {
        struct parser_event *event = session_read(client_session);

        int w_bytes = session_process(client_session);

        session_write(client_session, w_bytes);
    }
    close(clientSocket);

    return 0;
}