#include "wrapper-functions.h"
#include <stdio.h>
#include <stdlib.h>

ssize_t _recv(int sockfd, void *buf, size_t len, int flags) {
    ssize_t result;

    if ((result = recv(sockfd, buf, len, flags)) < 0) {
        perror("recv()");
        exit(EXIT_FAILURE);
    }
    return result;
}

ssize_t _send(int sockfd, const void *buf, size_t len, int flags) {
    ssize_t result;

    if ((result = send(sockfd, buf, len, flags)) < 0) {
        perror("sendv()");
        exit(EXIT_FAILURE);
    }
    return result;
}

int _socket(int domain, int type, int protocol) {
    int result;

    if ((result = socket(domain, type, protocol)) < 0) {
        perror("socket()");
        exit(EXIT_FAILURE);
    }
    return result;
}

int _bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    int result;

    if ((result = bind(sockfd, addr, addrlen)) < 0) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }
    return result;
}

int _listen(int sockfd, int backlog) {
    int result;

    if ((result = listen(sockfd, backlog)) < 0) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }
    return result;
}

int _setsockopt(int sockfd, int level, int optname, const void *optval,
                socklen_t optlen) {
    int result;

    if ((result = setsockopt(sockfd, level, optname, optval, optlen)) < 0) {
        perror("setsockopt()");
        exit(EXIT_FAILURE);
    }
    return result;
}

int _accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen) {
    int result;

    if ((result = accept(sockfd, addr, addrlen)) < 0) {
        perror("accept()");
        exit(EXIT_FAILURE);
    }
    return result;
}

void *_malloc(size_t size) {
    void *result;

    if ((result = malloc(size)) == NULL) {
        perror("malloc()");
        exit(EXIT_FAILURE);
    }
    return result;
}

pid_t _waitpid(pid_t pid, int *wstatus, int options) {
    pid_t result;

    if ((result = waitpid(pid, wstatus, options)) < 0) {
        perror("waitpid()");
        exit(EXIT_FAILURE);
    }
    return result;
}

pid_t _fork(void) {
    pid_t result;

    if ((result = fork()) < 0) {
        perror("fork()");
        exit(EXIT_FAILURE);
    }
    return result;
}