#include "request_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum parser_state {
    HEADER,
    VERSION,
    AUTH,
    REQID,
    COMMAND,
    SUCCESS,
    ERROR
} parser_state;

typedef int (*ParseFunc)(parser_state *state, char *key, char *value,
                         popcorn_request *request);

int parse_command(parser_state *state, char *key, char *value,
                  popcorn_request *request) {
    if (strcmp(key, "command") != 0)
        return 1;

    char *brk;
    char *sep = " ";
    char cmdbuff[256];
    strcpy(cmdbuff, value);

    char *command = strtok_r(cmdbuff, sep, &brk);
    strcpy(request->command, command);

    char *argument1 = strtok_r(NULL, sep, &brk);
    if (argument1 == NULL)
        return 0;

    strcpy(request->argument1, argument1);

    char *argument2 = strtok_r(NULL, sep, &brk);
    if (argument2 == NULL)
        return 0;

    strcpy(request->argument2, argument2);

    return 0;
}

int parse_request_id(parser_state *state, char *key, char *value,
                     popcorn_request *request) {
    if (strcmp(key, "req-id") != 0)
        return 1;

    int req_id = atoi(value);
    if (req_id < 0)
        return 1;

    request->req_id = req_id;

    *state = COMMAND;

    return 0;
}

int parse_version(parser_state *state, char *key, char *value,
                  popcorn_request *request) {
    if (strcmp(key, "version") != 0)
        return 1;

    int version_num = atoi(value);
    if (version_num != 1)
        return 1;

    request->version = version_num;

    *state = AUTH;

    return 0;
}

int parse_credentials(parser_state *state, char *key, char *value,
                      popcorn_request *request) {
    if (strcmp(key, "auth") != 0)
        return 1;

    char *brk;
    char *sep = ":";

    char varbuff[256];
    strcpy(varbuff, value);

    char *username = strtok_r(varbuff, sep, &brk);
    if (strlen(username) > NAME_SIZE)
        return 1;

    char *password = strtok_r(NULL, sep, &brk);
    if (strlen(password) > PASSWORD_SIZE)
        return 1;

    strcpy(request->username, username);
    strcpy(request->password, password);

    *state = REQID;

    return 0;
}

int parse_header(parser_state *state, char *key, char *value,
                 popcorn_request *request) {
    if (strcmp(key, "popcorn") == 0)
        *state = VERSION;

    return 0;
}

ParseFunc func_arr[5] = {
    &parse_header,      // HEADER
    &parse_version,     // VERSION
    &parse_credentials, // AUTH
    &parse_request_id,  // REQID
    &parse_command,     // COMMAND
};

static int get_entry(char *line, char *key, char *value) {
    char *token, *brk;

    token = strtok_r(line, ":\r\n", &brk);
    strcpy(key, token);

    // skip trailing whitespaces in between
    while (*brk == ' ')
        brk++;

    token = strtok_r(NULL, "\r\n", &brk);

    if (token != NULL)
        strcpy(value, token);

    return 0;
}

static int dispatch(parser_state *state, char *key, char *value,
                    popcorn_request *request) {
    return (*func_arr[*state])(state, key, value, request);
}

int parse_request(char *request_str, popcorn_request *request) {
    printf("parsing\n");
    char reqbuff[256];
    strcpy(reqbuff, request_str);

    char *sep = "\r\n";
    char *brk;
    char *line = strtok_r(reqbuff, sep, &brk);
    char key[256], value[256];

    parser_state state = HEADER;

    while (line != NULL) {
        if (get_entry(line, key, value) == 0) {
            dispatch(&state, key, value, request);
            line = strtok_r(NULL, sep, &brk);
            key[0] = '\0';
            value[0] = '\0';
        }
    }

    return 0;
}