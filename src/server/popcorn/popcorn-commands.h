#ifndef POPCORN_H
#define POPCORN_H

#define NAME_SIZE     16
#define PASSWORD_SIZE 16

struct popcorn_request {
    int version;
    char username[NAME_SIZE];
    char password[PASSWORD_SIZE];
    int req_id;
    char *command;
    char *argument1;
    char *argument2;
};

typedef enum {
    OK = 20,
    CLIENT_ERROR = 40,
    BAD_CREDENTIALS = 41,
    USER_NOT_EXISTS = 42,
    SERVER_ERROR = 50
} status_code;

struct popcorn_command {
    char *command;
};

void popcorn_get_bytes(char *response);

void popcorn_get_current(char *response);

void popcorn_get_history(char *response);

void popcorn_change_password(char *response);

void popcorn_delete_user(char *response);

#endif // POPCORN_H
