#include "server_adt.h"
#include "../selector/selector.h"
#include "../session/session.h"
#include "wrapper-functions.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#define PORT            1110

#define ARRAY_INCREMENT 2

#define OPTIONS_COUNT 1

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

// Used to check if all passwords are provided
static int registered_users_count = 0;

typedef int(*option_handler)(int argc, char *argv[]);

typedef struct option_t{
    char * option;
    option_handler handler;
} option_t;

int handle_user_option(int argc, char *argv[]){
    if (argc == 0){
        fprintf(stderr, "FATAL_ERROR: Format for -u is <user>:<pass>\n");
        return -1;
    }
    const char * delimiter = ":";
    char * username = strtok(argv[0], delimiter);
    if (username == NULL){
        fprintf(stderr, "FATAL_ERROR: Format for -u is <user>:<pass>\n");
        return -1;
    }
    int user_index = -1;
    for (int i=0; i < server->users_count; i++){
        if (strcmp(server->users_dir[i]->username, username) == 0){
            user_index = i;
        }
    }
    if (user_index == -1){
        fprintf(stderr, "FATAL_ERROR: No dir matches username \"%s\"\n", username);
        return -1;
    }
    char * password = strtok(NULL, delimiter);
    if (password == NULL){
        fprintf(stderr, "FATAL_ERROR: No password provided for username \"%s\"\n", username);
        return -1;
    }
    if (strlen(password) >= 16){
        fprintf(stderr, "FATAL_ERROR: Password for username \"%s\" is too long (16 characters max)\n", username);
        return -1;
    }
    strcpy(server->users_dir[user_index]->password, password);
    registered_users_count++;
    return 1;
}

option_t options[OPTIONS_COUNT] = {
    {.option = "-u", .handler = handle_user_option},
};

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
            (strncmp("..", mail_dirent->d_name, 2) == 0)) {
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
        users_dir[users_count] = _calloc(1, sizeof(struct user_dir));
        strncpy(users_dir[users_count]->username, mail_dirent->d_name, 256);
        users_count++;
        mail_dirent = readdir(mail_dir);
    }

    users_dir[users_count] = NULL;

    closedir(mail_dir);

    *count = users_count;
    return users_dir;
}

struct server *init_server(char *root_path, int argc, char *argv[]) {
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

    argv++;
    argc--;
    while (argc > 0){
        bool found_option = false;
        for (int i=0; i < OPTIONS_COUNT && !found_option; i++){
            if ( strcmp(argv[0], options[i].option) == 0 ){
                found_option = true;
                argv++;
                argc--;
                options[i].handler(argc, argv);
            }
        }
        if (!found_option){
            fprintf(stderr, "FATAL_ERROR: Invalid command \"%s\" \n", argv[0]);
            break;
        }
        argv++;
        argc--;
    }

    if (registered_users_count < server->users_count){
        fprintf(stderr, "FATAL_ERROR: Missing passwords for user directories\n");
    }

    server->clients = NULL;
    server->clients_count = 0;

    server->server_sock_handler = malloc(sizeof(fd_handler));

    return server;
}

int get_server_socket() {
    return server->server_sock;
}

struct user_dir * get_user_dir(char * username, int len){

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
        close_client_session(client_session);
        client_node *to_free = current;
        current = to_free->next;
        free(to_free);
    }
}

void close_server() {
    close(server->server_sock);
    free_clients();
    free_users_dir();
    free(server->server_sock_handler);
    free(server);
    server = NULL;
}
