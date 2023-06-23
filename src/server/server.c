#include "popcorn/popcorn-adt.h"
#include "popcorn/popcorn-handler.h"
#include "server_adt.h"
#include "session/session.h"
#include "utils.h"
#include "utils/logger.h"
#include "wrapper-functions.h"
#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_SELECTOR_FDS 500

static bool done = false;

static void sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n", signal);
    done = true;
}

static fd_selector server_init_selector(int ipv4_server_sock,
                                        int ipv6_server_sock,
                                        fd_handler *server_sock_handler,
                                        struct selector_init *conf);
void server_passive_accept(struct selector_key *key);

int main(int argc, char *argv[]) {

    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT, sigterm_handler);

    init_popcorn();
    init_server(argc, argv);

    int ipv4_server_sock = get_ipv4_server_socket();
    logv(INFO, "Got [%d] IPv4 server sock", ipv4_server_sock)

        int ipv6_server_sock = get_ipv6_server_socket();
    logv(INFO, "Got [%d] IPv6 server sock", ipv6_server_sock)

        set_server_sock_handlers(&server_passive_accept, NULL);
    struct fd_handler *server_sock_handler = malloc(sizeof(struct fd_handler));
    memcpy((void *)server_sock_handler, get_server_sock_fd_handler(),
           sizeof(struct fd_handler));

    int popcorn_ipv4_socket = get_popcorn_ipv4_server_sock();
    logv(INFO, "Got [%d] Popcorn UDP IPv4 server sock", popcorn_ipv4_socket)

        int popcorn_ipv6_socket = get_popcorn_ipv6_server_sock();
    logv(INFO, "Got [%d] Popcorn UDP IPv6 server sock", popcorn_ipv6_socket)

        set_popcorn_sock_handlers(&popcorn_read, NULL);
    struct fd_handler *popcorn_sock_handler = malloc(sizeof(struct fd_handler));
    memcpy((void *)popcorn_sock_handler, get_popcorn_sock_fd_handler(),
           sizeof(struct fd_handler));

    struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout =
            {
                .tv_sec = 10,
                .tv_nsec = 0,
            },
    };

    fd_selector selector = server_init_selector(
        ipv4_server_sock, ipv6_server_sock, server_sock_handler, &conf);
    if (selector == NULL) {
        log(FATAL, "Could not init selector")
    }

    selector_register(selector, popcorn_ipv4_socket, popcorn_sock_handler,
                      OP_READ, NULL);

    selector_register(selector, popcorn_ipv6_socket, popcorn_sock_handler,
                      OP_READ, NULL);

    while (!done) {
        selector_select(selector);
    }

    selector_destroy(selector);
    free(server_sock_handler);
    free(popcorn_sock_handler);
    return 0;
}

static fd_selector server_init_selector(int ipv4_server_sock,
                                        int ipv6_server_sock,
                                        fd_handler *server_sock_handler,
                                        struct selector_init *conf) {
    int ret_val = selector_init(conf);

    if (ret_val != SELECTOR_SUCCESS) {
        logv(ERROR, "selector_init(): %s", selector_error(ret_val))
    }

    if (selector_fd_set_nio(ipv4_server_sock) != 0) {
        log(ERROR, "selector_fd_set_nio()")
    }

    if (selector_fd_set_nio(ipv6_server_sock) != 0) {
        log(ERROR, "selector_fd_set_nio()")
    }

    fd_selector selector = selector_new(MAX_SELECTOR_FDS);
    selector_register(selector, ipv4_server_sock, server_sock_handler, OP_READ,
                      NULL);

    selector_register(selector, ipv6_server_sock, server_sock_handler, OP_READ,
                      NULL);

    return selector;
}

void server_passive_accept(struct selector_key *key) {
    int client_socket = acceptConnection(key->fd);

    if (server_is_full()) {
        logv(DEBUG,
             "New client connection attempt with socket [%d] rejected: Server "
             "is full",
             client_socket) close(client_socket);
        return;
    }

    logv(DEBUG, "New client with socket [%d]", client_socket)

        session_ptr client_session = new_client_session(client_socket);
    add_client(client_session);
    selector_register(key->s, client_socket, get_fd_handler(client_session),
                      OP_WRITE, client_session);
}