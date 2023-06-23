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

typedef struct popcorn_admin {
    char username[16];
    char password[16];
} popcorn_admin;

struct popcorn {
    int server_sock;

    popcorn_admin admin;

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

    popcorn_server = _calloc(1, sizeof(struct popcorn));
    popcorn_server->server_sock = server_sock;

    popcorn_server->sock_fd_handler = _malloc(sizeof(fd_handler));
    popcorn_server->sock_fd_handler->handle_close = close_popcorn_server_handler;

    return popcorn_server;
}

int get_popcorn_server_sock(void) {
    return popcorn_server->server_sock;
}

void set_popcorn_admin(char * username, char * password){
    strcpy(popcorn_server->admin.username, username);
    strcpy(popcorn_server->admin.password, password);
}

bool validate_user_pass(const char *user, const char *pass) {
    return strcmp(popcorn_server->admin.username, user) == 0 &&
           strcmp(popcorn_server->admin.password, pass) == 0;
}

struct fd_handler *get_popcorn_sock_fd_handler(void) {
    return popcorn_server->sock_fd_handler;
}

void set_popcorn_sock_handlers(void (*handle_read)(struct selector_key *key),
                               void (*handle_write)(struct selector_key *key)) {
    popcorn_server->sock_fd_handler->handle_read = handle_read;
    popcorn_server->sock_fd_handler->handle_write = handle_write;
}

void close_popcorn_server(){
    if(popcorn_server == NULL)
        return ;
    close(popcorn_server->server_sock);
    free(popcorn_server->sock_fd_handler);
    free(popcorn_server);
}

void close_popcorn_server_handler(struct selector_key * key){
    close_popcorn_server();
}
