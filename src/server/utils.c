#include "utils.h"
#include "parser/command_parser.h"
#include "session/session.h"
#include "sm/sm.h"
#include "wrapper-functions.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int setupIpv4ServerSocket(int port) {
    // IPv4 address
    // INADDR_ANY  (0.0.0.0)  means any address for binding
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    int ipv4_sock = _socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    _setsockopt(ipv4_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    _bind(ipv4_sock, (struct sockaddr *)&addr, sizeof(addr));
    _listen(ipv4_sock, MAX_QUEUED_CONNECTIONS);

    return ipv4_sock;
}

int setupIpv6ServerSocket(int port){

    struct sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = in6addr_any;
    addr.sin6_port = htons(port);

    int ipv6_sock = _socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP);

    _setsockopt(ipv6_sock, IPPROTO_IPV6, IPV6_V6ONLY, &(int){1}, sizeof(int));
    _setsockopt(ipv6_sock, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    _bind(ipv6_sock, (struct sockaddr *)&addr, sizeof(addr));
    _listen(ipv6_sock, MAX_QUEUED_CONNECTIONS);

    return ipv6_sock;
}

int acceptConnection(int serverSock) {
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    return _accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
}