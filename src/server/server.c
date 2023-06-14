#include "../selector/selector.h"
#include "../session/session.h"
#include "server_adt.h"
#include "utils.h"
#include "wrapper-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CURRENT_CLIENTS 500

static bool done = false;

static void sigterm_handler(const int signal) {
    printf("signal %d, cleaning up and exiting\n",signal);
    done = true;
}

void server_passive_accept(struct selector_key *key);

int main(int argc, char const *argv[]) {
    close(STDIN_FILENO);
    close(STDOUT_FILENO);

    signal(SIGTERM, sigterm_handler);
    signal(SIGINT,  sigterm_handler);

    server_ptr server = init_server("../mail");
    int server_sock = get_server_socket();

    const struct selector_init conf = {
        .signal = SIGALRM,
        .select_timeout = {
            .tv_sec  = 10,
            .tv_nsec = 0,
        },
    };
    if(0 != selector_init(&conf)) {
        //TODO: HANDLE ERROR
    }
    selector_fd_set_nio(server_sock);
    fd_selector selector = selector_new(1024);
    const struct fd_handler server_sock_handler = {
        .handle_read       = server_passive_accept,
        .handle_write      = NULL,
        .handle_close      = NULL, // nada que liberar
    };
    selector_register(selector,server_sock,&server_sock_handler,OP_READ,NULL);


    while(!done){
        selector_select(selector);
    }
}

void server_passive_accept(struct selector_key *key){
    int client_socket = acceptConnection(key->fd);
    session_ptr client_session = new_client_session(client_socket);
    selector_register(key->s,client_socket,get_fd_handler(client_session),OP_READ,client_session);
}
