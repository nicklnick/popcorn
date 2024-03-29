#include "server_adt.h"
#include "./popcorn/popcorn-adt.h"
#include "selector/selector.h"
#include "session/session.h"
#include "utils/file-utils.h"
#include "utils/logger.h"
#include "wrapper-functions.h"
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT                   1110
#define MAX_CONCURRENT_CLIENTS 500
#define ARRAY_INCREMENT        2

typedef struct client_node {
    session_ptr client;
    struct client_node *next;
} client_node;

struct server {
    int ipv4_server_sock;
    int ipv6_server_sock;

    struct user_dir **users_dir;
    int users_count;

    char *root_path;

    client_node *clients;
    int clients_count;

    int max_concurrent_clients;
    long transferred_bytes_count;
    int historic_client_count;

    struct fd_handler *server_sock_handler;
};

struct server *server = NULL;

static void register_user_admin(int argc, char *argv[]) {
    if (argc == 0) {
        log(FATAL, "-a: Usage: -a <user>:<password>")
    }
    const char *delimiter = ":";
    char *username = strtok(argv[0], delimiter);
    if (username == NULL) {
        log(FATAL, "-a: Usage: -a <user>:<password>")
    }
    if (strlen(username) >= 16) {
        log(FATAL, "-a: Username is too long (16 characters max)")
    }
    char *password = strtok(NULL, delimiter);
    if (password == NULL) {
        log(FATAL, "Usage: -a <user>:<password>")
    }
    if (strlen(password) >= 16) {
        log(FATAL, "-a: Password for username is too long (16 characters max)")
    }
    set_popcorn_admin(username, password);
}

static void register_user_pass(int argc, char *argv[]) {
    if (argc == 0) {
        log(FATAL, "-u: Format is -u <user>:<password>")
    }
    const char *delimiter = ":";
    char *username = strtok(argv[0], delimiter);
    if (username == NULL) {
        log(FATAL, "-u: Format is -u <user>:<password>")
    }
    int user_index = -1;
    for (int i = 0; i < server->users_count; i++) {
        if (strcmp(server->users_dir[i]->username, username) == 0) {
            user_index = i;
        }
    }
    if (user_index == -1) {
        logv(FATAL, "-u: No directory matches username \"%s\"", username)
    }
    char *password = strtok(NULL, delimiter);
    if (password == NULL) {
        logv(FATAL, "-u: No password provided for username \"%s\"", username)
    }
    if (strlen(password) >= 16) {
        logv(FATAL,
             "-u: Password for username \"%s\" is too long (16 characters max)",
             username)
    }
    strcpy(server->users_dir[user_index]->password, password);
}

static int register_port(int argc, char *argv[]) {
    if (argc == 0) {
        log(FATAL, "-p: Format is -p <port>");
    }
    return atoi(argv[0]);
}

static void init_users_dir(char *root_path) {
    log(INFO, "Initializing users_dir")

        DIR *mail_dir = opendir(root_path);
    if (mail_dir == NULL) {
        log(FATAL, "opendir()")
    }

    server->root_path = root_path;

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

    server->users_count = users_count;
    server->users_dir = users_dir;
}

struct server *init_server(int argc, char *argv[]) {
    if (server != NULL)
        return server;

    if (argc <= 1) {
        log(FATAL, "Missing mail directory and users\n"
                   "Usage: ./server [-p <port>] -d <mail_path> -a "
                   "<user:password> -u <user:password> [-u "
                   "user:password]...\n");
    }
    int server_port = PORT;

    server = calloc(1, sizeof(struct server));

    server->clients = NULL;
    server->clients_count = 0;
    server->historic_client_count = 0;
    server->max_concurrent_clients = MAX_CONCURRENT_CLIENTS;

    server->server_sock_handler = malloc(sizeof(fd_handler));
    server->server_sock_handler->handle_close = close_server;

    argv++;
    argc--;
    bool mail_dir_set = false;
    bool admin_set = false;
    bool port_set = false;
    int registered_users_count = 0;
    while (argc > 0) {
        if (strcmp(argv[0], "-d") == 0) {
            if (!mail_dir_set) {
                argc--;
                argv++;
                if (argc == 0) {
                    logv(FATAL, "%s: Format is %s mail_path", "-d", "-d")
                }
                init_users_dir(argv[0]);
                mail_dir_set = true;
            } else {
                logv(FATAL, "%s: Mail directory already set", "-d");
            }
        } else if (strcmp(argv[0], "-u") == 0) {
            if (!mail_dir_set) {
                logv(FATAL, "%s: Mail directory needs to be specified first",
                     "-d")
            }
            argc--;
            argv++;
            register_user_pass(argc, argv);
            registered_users_count++;
        } else if (strcmp(argv[0], "-a") == 0) {
            if (!mail_dir_set) {
                logv(FATAL, "%s: Mail directory needs to be specified first",
                     "-d")
            }
            if (admin_set) {
                logv(FATAL, "%s: Admin user already set", "-a")
            }
            admin_set = true;
            argc--;
            argv++;
            register_user_admin(argc, argv);
        } else if (strcmp(argv[0], "-p") == 0) {
            if (port_set) {
                logv(FATAL, "%s: Port was already specified", "-p")
            }
            port_set = true;
            argc--;
            argv++;
            server_port = register_port(argc, argv);
        } else {
            logv(FATAL, "Invalid command \"%s\"", argv[0])
        }
        argv++;
        argc--;
    }

    if (registered_users_count < server->users_count) {
        logv(FATAL, "%s: Missing passwords for mail directory", "-u")
    }

    if (!admin_set) {
        logv(FATAL, "%s: Missing admin user for monitoring protocol", "-a")
    }

    int ipv4_server_sock = setupIpv4ServerSocket(server_port);
    int ipv6_server_sock = setupIpv6ServerSocket(server_port);

    server->ipv4_server_sock = ipv4_server_sock;
    server->ipv6_server_sock = ipv6_server_sock;

    if (ipv4_server_sock < 0) {
        log(ERROR, SETUP_SERVER_SOCKET_ERROR) return NULL;
    }

    return server;
}

int get_ipv4_server_socket(void) {
    return server->ipv4_server_sock;
}

int get_ipv6_server_socket(void) {
    return server->ipv6_server_sock;
}

struct user_dir *get_user_dir(char *username, int len) {
    if (server == NULL)
        return NULL;

    int i = 0;
    struct user_dir *user_dir;

    while (server->users_dir[i] != NULL) {
        if (strncmp(username, server->users_dir[i]->username, len) == 0 &&
            !server->users_dir[i]->removed) {
            user_dir = server->users_dir[i];
            return user_dir;
        }
        i++;
    }

    return NULL;
}

int delete_user_dir(char *username, int len) {
    const char *mail_dir_path = get_mail_dir_path();
    DIR *mail_dir = opendir(mail_dir_path);
    int status = 0;

    struct dirent *entry;
    while ((entry = readdir(mail_dir)) != NULL) {
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, username) == 0) {
                char user_mail_path[PATH_MAX] = {0};
                strcpy(user_mail_path, get_mail_dir_path());
                strcat(user_mail_path, "/");
                strncat(user_mail_path, username, len);

                DIR *user_dir = opendir(user_mail_path);
                if (user_dir != NULL) {
                    delete_files_from_dir(user_dir, user_mail_path);
                    if (rmdir(user_mail_path) == 0) {
                        logv(DEBUG, "Deleted directory: %s", user_mail_path)
                    }

                    if (get_file_count(user_dir) == 0) {
                        status = 0;
                    } else {
                        status = 1;
                    }

                    closedir(user_dir);
                }

                break;
            }
        }
    }

    closedir(mail_dir);

    return status;
}

int set_max_concurrent_clients(int max_count) {
    if (server->clients_count > max_count)
        return 1;

    server->max_concurrent_clients = max_count;
    return 0;
}

int server_is_full() {
    return server->clients_count == server->max_concurrent_clients;
}

char *get_mail_dir_path(void) {
    return server->root_path;
}

unsigned int get_historic_client_count(void) {
    if (server == NULL)
        return 0;

    return server->historic_client_count;
}

unsigned int get_clients_count(void) {
    if (server == NULL)
        return 0;

    return server->clients_count;
}

unsigned long get_transferred_bytes(void) {
    if (server == NULL)
        return 0;

    return server->transferred_bytes_count;
}

void add_transferred_bytes(unsigned int nbytes) {
    server->transferred_bytes_count += nbytes;
}

struct fd_handler *get_server_sock_fd_handler(void) {
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
            log(ERROR, "malloc()") return -1;
        }
        current->next = NULL;
        current->client = client;
        server->clients = current;
        server->clients_count++;
        server->historic_client_count++;
        return 0;
    }

    client_node *prev = NULL;
    while (current != NULL) {
        prev = current;
        current = current->next;
    }
    current = malloc(sizeof(client_node));
    if (current == NULL) {
        log(ERROR, "malloc()") return -1;
    }
    current->next = NULL;
    prev->next = current;
    current->client = client;
    server->clients_count++;
    server->historic_client_count++;
    return 0;
}

int remove_client(session_ptr client) {
    if (server == NULL)
        return 0;
    client_node *prev = server->clients;
    if (prev == NULL) {
        return -1;
    }
    client_node *current = prev->next;
    if (current == NULL) {
        if (prev->client == client) {
            server->clients = NULL;
            server->clients_count--;
            free(prev);
            return 0;
        }
    }
    while (current != NULL) {
        if (current->client == client) {
            client_node *to_free = current;
            prev->next = current->next;
            server->clients_count--;
            free(to_free);
            return 0;
        }
        prev = current;
        current = prev->next;
    }
    return -1;
}

static void free_users_dir(void) {
    struct user_dir **users_dir = server->users_dir;
    for (int i = 0; i < server->users_count; ++i) {
        free(users_dir[i]);
    }
    free(users_dir);
}

static void free_clients(void) {
    client_node *current = server->clients;
    while (current != NULL) {
        client_node *to_free = current;
        current = to_free->next;
        free(to_free);
    }
}

void close_server() {
    if (server == NULL)
        return;
    close(server->ipv4_server_sock);
    close(server->ipv6_server_sock);
    free_clients();
    if (server->users_dir != NULL) {
        free_users_dir();
    }
    free(server->server_sock_handler);
    free(server);
    server = NULL;
}
