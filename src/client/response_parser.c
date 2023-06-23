#include "response_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

typedef enum parser_state {
    HEADER,
    VERSION,
    REQID,
    STATUS_CODE,
    RESPONSE_VALUE,
    SUCCESS,
    ERROR
} parser_state;

typedef int (*ParseFunc)(parser_state *state, char *key, char *value,
                         popcorn_response *response);

int parse_header(parser_state *state, char *key, char *value,
                 popcorn_response *response) {

    if (strcmp(key, "popcorn") != 0){
        return 1;
    }

    *state = VERSION;
    return 0;
}

int parse_version(parser_state *state, char *key, char *value,
                  popcorn_response *response) {
    if (strcmp(key, "version") != 0)
        return 1;

    int version_num = atoi(value);
    if (version_num != 1)
        return 1;

    response->version = version_num;

    *state = REQID;

    return 0;
}

int parse_request_id(parser_state *state, char *key, char *value,
                     popcorn_response *response) {
    if (strcmp(key, "req-id") != 0)
        return 1;

    int req_id = atoi(value);
    if (req_id < 0)
        return 1;

    response->req_id = req_id;

    *state = STATUS_CODE;

    return 0;
}

int parse_status_code(parser_state *state, char *key, char *value,
                 popcorn_response *response) {

    if (strcmp(key, "status") != 0)
        return 1;
    
    response->status_code = atoi(value);
    *state = RESPONSE_VALUE;

    return 0;
}

int parse_value(parser_state *state, char *key, char *value,
                 popcorn_response *response) {

    if (key == NULL)
        return 0;
    
    if (strcmp(key, "value") != 0)
        return 1;      

    strcpy(response->response_value, value);

    return 0;
}

ParseFunc func_arr[5] = {
    &parse_header,      // HEADER
    &parse_version,     // VERSION
    &parse_request_id,  // REQID
    &parse_status_code, // STATUS_CODE
    &parse_value,       // VALUE
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
                    popcorn_response *request) {
    return (*func_arr[*state])(state, key, value, request);
}

int parse_response(char *response_str, popcorn_response *response) {
    char reqbuff[BUFFER_SIZE];
    strcpy(reqbuff, response_str);

    char *sep = "\r\n";
    char *brk;
    char *line = strtok_r(reqbuff, sep, &brk);
    char key[RES_COMMAND_SIZE], value[RES_VALUE_SIZE];

    parser_state state = HEADER;

    while (line != NULL) {
        if (get_entry(line, key, value) == 0) {
            dispatch(&state, key, value, response);
            line = strtok_r(NULL, sep, &brk);
            key[0] = '\0';
            value[0] = '\0';
        }
    }

    return 0;
}
