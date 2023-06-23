#ifndef REQUEST_PARSER_H
#define REQUEST_PARSER_H

#define RES_BUFF_SIZE    256
#define NAME_SIZE        16
#define PASSWORD_SIZE    16
#define REQ_COMMAND_SIZE 20

typedef struct popcorn_request {
    int version;
    char username[NAME_SIZE + 1];
    char password[PASSWORD_SIZE + 1];
    int req_id;
    char command[REQ_COMMAND_SIZE + 1];
    char argument1[REQ_COMMAND_SIZE + 1];
    char argument2[REQ_COMMAND_SIZE + 1];
} popcorn_request;

typedef enum {
    OK = 20,
    CLIENT_ERROR = 40,
    BAD_CREDENTIALS = 41,
    USER_NOT_EXISTS = 42,
    USER_LOGGED_IN = 43,
    VERSION_NOT_SUPPORTED = 49,
    SERVER_ERROR = 50
} status_code;

typedef struct popcorn_response {
    int version;
    int req_id;
    char value[256];
    status_code status;
} popcorn_response;

int parse_request(char *request_str, popcorn_request *request);

#endif
