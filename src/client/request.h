#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NAME_SIZE     16
#define PASSWORD_SIZE 16
#define OPTION_SIZE   20
#define COMMAND_SIZE  20
#define ARGUMENT_SIZE 20

#define TOTAL_OPTIONS 6

#define SUCCESS       0
#define ERROR         -1

#define error(...)                                                             \
    {                                                                          \
        fprintf(stderr, "client: ");                                           \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
    }

#define error_and_exit(...)                                                    \
    {                                                                          \
        fprintf(stderr, "client: ");                                           \
        fprintf(stderr, __VA_ARGS__);                                          \
        fprintf(stderr, "\n");                                                 \
        exit(ERROR);                                                           \
    }

struct request *get_request_struct(int version, int argc, char *argv[]);

void get_request(char *buffer, struct request *request);
