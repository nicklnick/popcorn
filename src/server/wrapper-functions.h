#ifndef _WRAPPER_FUNCTIONS_H_
#define _WRAPPER_FUNCTIONS_H_

#include <sys/socket.h>
#include <sys/types.h>

/**
 * Wrapper functions to ensure correct error handling
 */
ssize_t _recv(int sockfd, void *buf, size_t len, int flags);
ssize_t _send(int sockfd, const void *buf, size_t len, int flags);
int _socket(int domain, int type, int protocol);
int _bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int _listen(int sockfd, int backlog);
int _setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
int _accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);

#endif  // _WRAPPER_FUNCTIONS_H_