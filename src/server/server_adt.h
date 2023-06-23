#ifndef SERVER_ADT_H
#define SERVER_ADT_H

#include "selector/selector.h"
#include "session/session.h"
#include "utils.h"
#include <limits.h>

typedef struct server *server_ptr;

struct user_dir {
    char username[NAME_MAX];
    char password[16];
    bool is_open;
};

server_ptr init_server(int argc, char *argv[]);

struct user_dir *get_user_dir(char *username, int len);

server_ptr get_server_instance(void);

int get_ipv4_server_socket(void);

int get_ipv6_server_socket(void);

char *get_mail_dir_path(void);

unsigned int get_historic_client_count(void);

unsigned int get_clients_count(void);

unsigned long get_transferred_bytes(void);

void add_transferred_bytes(unsigned int nbytes);

void close_server();

struct fd_handler *get_server_sock_fd_handler(void);

void set_server_sock_handlers(void (*handle_read)(struct selector_key *key),
                              void (*handle_write)(struct selector_key *key));

int add_client(session_ptr client);

int remove_client(session_ptr client);

#endif // SERVER_ADT_H
