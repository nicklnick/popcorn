#include "popcorn-adt.h"
#include "../utils.h"
#include "../wrapper-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// hours to grow corn + min to microwave them (average)
#define POPCORN_PORT 2882

#define USER_PASS    2
#define USER         0
#define PASS         1
#define ADMIN_USER   "pop"
#define ADMIN_PASS   "corn"

struct popcorn {
    int server_sock;
    char *auth[USER_PASS];

    struct fd_handler *sock_fd_handler;
};

struct popcorn *popcorn_server = NULL;

struct popcorn *init_popcorn(void) {

    if (popcorn_server != NULL)
        return popcorn_server;

    int server_sock = setup_udp_ipv4_socket(POPCORN_PORT);
    if (server_sock < 0) {
        perror(SETUP_SERVER_SOCKET_ERROR);
        return NULL;
    }

    popcorn_server = _malloc(sizeof(struct popcorn));
    popcorn_server->server_sock = server_sock;

    // save user and pass
    /*popcorn_server->auth[USER] =
        (char *)_calloc(strlen(ADMIN_USER), sizeof(char));
    strcpy(popcorn_server->auth[USER], ADMIN_USER);
    popcorn_server->auth[PASS] =
        (char *)_calloc(strlen(ADMIN_PASS), sizeof(char));
    strcpy(popcorn_server->auth[PASS], ADMIN_PASS);*/

    popcorn_server->sock_fd_handler = _malloc(sizeof(fd_handler));

    return popcorn_server;
}

int get_popcorn_server_sock(void) {
    return popcorn_server->server_sock;
}

int validate_user_pass(const char *user, const char *pass) {
    return strcmp(user, popcorn_server->auth[USER]) == 0 &&
           strcmp(pass, popcorn_server->auth[PASS]) == 0;
}

struct fd_handler *get_popcorn_sock_fd_handler(void) {
    return popcorn_server->sock_fd_handler;
}

void set_popcorn_sock_handlers(void (*handle_read)(struct selector_key *key),
                               void (*handle_write)(struct selector_key *key)) {
    popcorn_server->sock_fd_handler->handle_read = handle_read;
    popcorn_server->sock_fd_handler->handle_write = handle_write;
}
