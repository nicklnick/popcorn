#include "server_adt.h"
#include "../selector/selector.h"
#include "../session/session.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT            1110

#define ARRAY_INCREMENT 2

typedef struct client_node {
    session_ptr client;
    struct client_node *next;
} client_node;

struct server {
    int server_sock;

    struct user_dir **users_dir;
    int users_count;

    char *root_path;

    client_node *clients;
    int clients_count;

    struct fd_handler *server_sock_handler;
};

struct server *server = NULL;

static struct user_dir **init_users_dir(char *root_path, int *count) {
    DIR *mail_dir = opendir(root_path);
    if (mail_dir == NULL) {
        perror("opendir()");
        exit(1);
    }

    int users_count = 0;
    struct user_dir **users_dir;

    struct dirent *mail_dirent = readdir(mail_dir);
    users_dir = calloc((ARRAY_INCREMENT + 1), sizeof(struct user_dir *));
    while (mail_dirent != NULL) {
        if ((strncmp(".", mail_dirent->d_name, 1) == 0) ||
            (strncmp("..", mail_dirent->d_name, 1) == 0)) {
            mail_dirent = readdir(mail_dir);
            continue;
        }

        if (users_count > 0 && (users_count % ARRAY_INCREMENT) == 0) {
            users_dir =
                realloc(users_dir, (users_count + (ARRAY_INCREMENT + 1)) *
                                       sizeof(struct user_dir *));
            memset(users_dir + users_count, 0,
                   ARRAY_INCREMENT * sizeof(struct user_dir *));
        }
        users_dir[users_count] = calloc(1, sizeof(struct user_dir));
        strncpy(users_dir[users_count]->username, mail_dirent->d_name, 256);
        users_count++;
        mail_dirent = readdir(mail_dir);
    }

    users_dir[users_count] = NULL;

    closedir(mail_dir);

    *count = users_count;
    return users_dir;
}

struct server *init_server(char *root_path) {

    if (server != NULL)
        return server;

    int server_sock = setupServerSocket(PORT);
    if (server_sock < 0) {
        perror(SETUP_SERVER_SOCKET_ERROR);
        return NULL;
    }

    server = malloc(sizeof(struct server));
    server->server_sock = server_sock;
    server->root_path = root_path;
    server->users_dir = init_users_dir(root_path, &server->users_count);

    server->clients = NULL;
    server->clients_count = 0;

    server->server_sock_handler = malloc(sizeof(fd_handler));
}

int get_server_socket() {
    return server->server_sock;
}

struct user_dir *get_user_dir(char *username, int len) {

    int i = 0;
    struct user_dir *user_dir;

    while (server->users_dir[i] != NULL) {
        if (strncmp(username, server->users_dir[i]->username, len) == 0) {
            user_dir = server->users_dir[i];
            return user_dir;
        }
        i++;
    }

    return NULL;
}

char *get_mail_dir_path() {
    return server->root_path;
}

struct fd_handler *get_server_sock_fd_handler() {
    return server->server_sock_handler;
}

void set_server_sock_handlers(void (*handle_read)(struct selector_key *key),
                              void (*handle_write)(struct selector_key *key)) {
    server->server_sock_handler->handle_read = handle_read;
    server->server_sock_handler->handle_write = handle_write;
}

int add_client(session_ptr client) {
    client_node *current = server->clients;
    if (current == NULL) {
        current = malloc(sizeof(client_node));
        if (current == NULL) {
            return -1;
        }
        current->next = NULL;
        current->client = client;
        server->clients = current;
        server->clients_count++;
        return 0;
    }

    client_node *prev = NULL;
    while (current != NULL) {
        prev = current;
        current = current->next;
    }
    current = malloc(sizeof(client_node));
    if (current == NULL) {
        return -1;
    }
    current->next = NULL;
    prev->next = current;
    current->client = client;
    server->clients_count++;
    return 0;
}

static void free_users_dir() {
    struct user_dir **users_dir = server->users_dir;
    for (int i = 0; i < server->users_count; ++i) {
        free(users_dir[i]);
    }
    free(users_dir);
}

static void free_clients() {
    client_node *current = server->clients;
    while (current != NULL) {
        session_ptr client_session = current->client;
        // close_client_session(client_session);
        client_node *to_free = current;
        current = to_free->next;
        free(to_free);
    }
}

void close_server() {
    close(server->server_sock);
    free_clients();
    free_users_dir();
    free(server);
    server = NULL;
}