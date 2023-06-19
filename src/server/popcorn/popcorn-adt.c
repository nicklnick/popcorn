#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "../utils.h"
#include "../wrapper-functions.h"

#define POPCORN_PORT 2882   // hours to grow corn + min to microwave them (average)

#define USER_PASS 2
#define USER 0
#define PASS 1
#define ADMIN_USER "pop"
#define ADMIN_PASS "corn"

struct popcorn {
    int server_sock;
    char* auth[USER_PASS];
};

struct popcorn *popcorn_server = NULL;

struct popcorn *init_popcorn(void) {
    if(popcorn_server != NULL)
        return popcorn_server;

    int server_sock = setup_udp_ipv4_socket(POPCORN_PORT);
    if(server_sock < 0) {
        perror(SETUP_SERVER_SOCKET_ERROR);
        return NULL;
    }

    popcorn_server = _malloc(sizeof(struct popcorn));
    popcorn_server->server_sock = server_sock;
    // save user and pass
    popcorn_server->auth[USER] = (char*)_calloc(strlen(ADMIN_USER), sizeof(char));
    strcpy(popcorn_server->auth[USER], ADMIN_USER);
    popcorn_server->auth[PASS] = (char*)_calloc(strlen(ADMIN_PASS), sizeof(char));
    strcpy(popcorn_server->auth[PASS], ADMIN_PASS);

    return popcorn_server;
}

int get_popcorn_server_socket(void) {
    return popcorn_server->server_sock;
}

int validate_user_pass(const char* user, const char* pass) {
    return strcmp(user, popcorn_server->auth[USER]) == 0
           && strcmp(pass, popcorn_server->auth[PASS]) == 0;
}
