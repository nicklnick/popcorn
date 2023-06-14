#ifndef SERVER_ADT_H
#define SERVER_ADT_H

#include "utils.h"
#include <limits.h>

typedef struct server *server_ptr;

struct user_dir {
    char username[NAME_MAX];
    char password[16];
    bool is_open;
};

server_ptr init_server(char *root_path);

struct user_dir *get_user_dir(char *username, int len);

server_ptr get_server_instance();

int get_server_socket();

char *get_mail_dir_path();

void close_server();

#endif // SERVER_ADT_H
