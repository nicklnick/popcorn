#include "popcorn-commands.h"
#include "../server_adt.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MAX_RESPONSE_LENGTH 128

typedef void (*command_function)(char *argument1, char *argument2,
                                 popcorn_response *response);

void popcorn_get_bytes(char *argument1, char *argument2,
                       popcorn_response *response) {
    unsigned long transferred_bytes_count = get_transferred_bytes();
    snprintf(response->value, 256, "%ld", transferred_bytes_count);
    response->status = OK;
}

void popcorn_get_current(char *argument1, char *argument2,
                         popcorn_response *response) {
    int current_clients_count = get_clients_count();
    snprintf(response->value, 256, "%d", current_clients_count);
    response->status = OK;
}

void popcorn_get_history(char *argument1, char *argument2,
                         popcorn_response *response) {
    int historic_clients_count = get_historic_client_count();
    snprintf(response->value, 256, "%d", historic_clients_count);
}

void popcorn_change_password(char *argument1, char *argument2,
                             popcorn_response *response) {
    char *username = argument1, *password = argument2;

    if (username == NULL || password == NULL) {
        response->status = CLIENT_ERROR;
        return;
    }

    if (strlen(password) > PASSWORD_SIZE) {
        response->status = CLIENT_ERROR;
        return;
    }

    struct user_dir *user_dir = get_user_dir(username, strlen(username));
    if (user_dir == NULL) {
        response->status = USER_NOT_EXISTS;
        return;
    }

    if (user_dir->is_open) {
        response->status = USER_LOGGED_IN;
        return;
    }

    strcpy(user_dir->password, password);

    response->status = OK;
}

void popcorn_delete_user(char *argument1, char *argument2,
                         popcorn_response *response) {
    puts("popcorn_delete_user");
}

void popcorn_set_concurrent_users(char *argument1, char *argument2,
                                  popcorn_response *response) {
    puts("popcorn_set_concurrent_users");
}

typedef struct command_entry {
    char *command;
    command_function function;
} command_entry;

command_entry command_table[] = {{"bytes", &popcorn_get_bytes},
                                 {"history", &popcorn_get_history},
                                 {"current", &popcorn_get_current},
                                 {"password", &popcorn_change_password},
                                 {"delete", &popcorn_delete_user},
                                 {"conc", &popcorn_set_concurrent_users},
                                 {NULL, NULL}};

command_function get_command_function(char *command) {
    for (int i = 0; command_table[i].command != NULL; i++) {
        if (!strcmp(command, command_table[i].command)) {
            return command_table[i].function;
        }
    }

    return NULL;
}

void handle_request(popcorn_request *request, popcorn_response *response) {
    command_function cfunc = get_command_function(request->command);

    if (cfunc == NULL)
        puts("Command not found");

    response->req_id = request->req_id;
    response->version = request->version;

    cfunc(request->argument1, request->argument2, response);
}