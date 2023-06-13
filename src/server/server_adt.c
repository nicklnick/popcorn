#include "server_adt.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define PORT            110

#define ARRAY_INCREMENT 2

struct server {
    struct user_dir **users_dir;
    int users_count;
    int server_sock;

    char *root_path;
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
    while (mail_dirent != NULL) {
        if ((strncmp(".", mail_dirent->d_name, 1) == 0) ||
            (strncmp("..", mail_dirent->d_name, 1) == 0)) {
            mail_dirent = readdir(mail_dir);
            continue;
        }

        if ((users_count % ARRAY_INCREMENT) == 0) {
            users_dir = realloc(users_dir, (users_count + ARRAY_INCREMENT) *
                                               sizeof(struct user_dir *));
        }
        users_dir[users_count] = malloc(sizeof(struct user_dir));
        strncpy(users_dir[users_count]->username, mail_dirent->d_name, 256);
        users_count++;
        mail_dirent = readdir(mail_dir);
    }

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
}

int get_server_socket() {
    return server->server_sock;
}

struct user_dir * get_user_dir(char * username, int len){

    int i = 0;
    struct user_dir * user_dir;

    while(server->users_dir[i] != NULL){
        if(strncmp(username,server->users_dir[i]->username,len) == 0){
            user_dir = server->users_dir[i];
            return user_dir;
        }
        i++;
    }

    return NULL;
}