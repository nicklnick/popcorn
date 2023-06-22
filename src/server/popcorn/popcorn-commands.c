#include "popcorn-commands.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#define MAX_RESPONSE_LENGTH 128

typedef void (*command_function)(char *argument1, char *argument2,
                                 popcorn_response *response);

void popcorn_get_bytes(char *argument1, char *argument2,
                       popcorn_response *response) {
    puts("popcorn_get_bytes");
}

void popcorn_get_current(char *argument1, char *argument2,
                         popcorn_response *response) {
    snprintf(response->value, 256, "%d", 10);
}

void popcorn_get_history(char *argument1, char *argument2,
                         popcorn_response *response) {
    puts("popcorn_get_history");
}

void popcorn_change_password(char *argument1, char *argument2,
                             popcorn_response *response) {
    puts("popcorn_change_password");
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

    response->status = OK;
}