#ifndef SERVER_ADT_H
#define SERVER_ADT_H

#include "../selector/selector.h"
#include "../session/session.h"
#include "utils.h"
#include <limits.h>

typedef struct server *server_ptr;

struct user_dir {
    char username[NAME_MAX];
    char password[16];
    bool is_open;
};

server_ptr init_server(char *root_path, int argc, char *argv[]);

struct user_dir *get_user_dir(char *username, int len);

server_ptr get_server_instance();

int get_server_socket();

char *get_mail_dir_path();

void close_server();

struct fd_handler *get_server_sock_fd_handler();

void set_server_sock_handlers(void (*handle_read)(struct selector_key *key),
                              void (*handle_write)(struct selector_key *key));

int add_client(session_ptr client);

int remove_client(session_ptr client);

#endif // SERVER_ADT_H
