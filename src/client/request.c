#include "request.h"

struct request {
    int version;
    char username[NAME_SIZE];
    char password[PASSWORD_SIZE];
    int req_id;
    char command[COMMAND_SIZE];
    char argument1[ARGUMENT_SIZE];
    char argument2[ARGUMENT_SIZE];
} request;

typedef int (*handle_arguments)(int argc, char *argv[],
                                struct request *request);

struct option {
    char option[OPTION_SIZE];
    handle_arguments handler;
} option;

static void handle_help() {
    printf("POPCORN protocol client\n");
    printf("Usage: ./client -a <user>:<password> <command> [<args>]...\n\n");
    printf("Commands:\n");
    printf("  bytes\n");
    printf("  history\n");
    printf("  current\n");
    printf("  password <user> <password>\n");
    printf("  delete   <user>\n");
    printf("  conc     <max_users>\n");
}

static int handle_auth(int argc, char *argv[], struct request *request) {
    if (argc < 1) {
        error_and_exit("-a: missing username and password")
    }
    char *username = strtok(argv[0], ":");
    if (username == NULL) {
        error_and_exit("-a: format for authentication is user:password")
    }
    char *password = strtok(NULL, ":");
    if (password == NULL) {
        error_and_exit("-a: no password provided for username \"%s\"", username)
    }
    if (strlen(username) > NAME_SIZE) {
        error_and_exit("-a: username length is too long")
    }
    if (strlen(password) > PASSWORD_SIZE) {
        error_and_exit("-a: password size is too long")
    }
    strcpy(request->username, username);
    strcpy(request->password, password);
    return 1;
}

static int handle_bytes(int argc, char **argv, struct request *request) {
    if (argc >= 1) {
        error_and_exit("bytes: must not receive any arguments")
    }
    strcpy(request->command, "bytes");
    return 0;
}

static int handle_history(int argc, char **argv, struct request *request) {
    if (argc >= 1) {
        error_and_exit("history: must not receive any arguments")
    }
    strcpy(request->command, "history");
    return 0;
}

static int handle_current(int argc, char **argv, struct request *request) {
    if (argc >= 1) {
        error_and_exit("current: must not receive any arguments")
    }
    strcpy(request->command, "current");
    return 0;
}

static int handle_password(int argc, char **argv, struct request *request) {
    if (argc < 2) {
        error_and_exit("password: missing arguments")
    }
    if (argc > 2) {
        error_and_exit("password: too many arguments")
    }

    char *username = argv[0];
    char *password = argv[1];

    if (strlen(username) > NAME_SIZE) {
        error_and_exit("-password: username length is too long")
    }

    if (strlen(username) > PASSWORD_SIZE) {
        error_and_exit("-password: new password length is too long")
    }

    strcpy(request->command, "password");

    strcpy(request->argument1, username);
    strcpy(request->argument2, password);

    return 0;
}

static int handle_delete(int argc, char **argv, struct request *request) {
    if (argc < 1) {
        error_and_exit("delete: missing arguments")
    }
    if (argc > 1) {
        error_and_exit("delete: too many arguments")
    }

    strcpy(request->command, "delete");

    char *username = argv[0];

    if (strlen(username) > NAME_SIZE) {
        error_and_exit("delete: username length is too long")
    }

    strcpy(request->argument1, username);

    return 0;
}

static int handle_conc(int argc, char **argv, struct request *request) {
    if (argc < 1) {
        error_and_exit("conc: missing arguments")
    }
    if (argc > 1) {
        error_and_exit("conc: too many arguments")
    }

    strcpy(request->command, "conc");

    char * ptr;

    int num = strtol(argv[0], &ptr, 10);
    if (num <= 0 || num > CONC_MAX_VALUE || *ptr){
        error_and_exit("conc: argument must be a number between 1 and 1000")
    }

    strcpy(request->argument1, argv[0]);

    return 0;
}

struct option options[TOTAL_OPTIONS] = {
    {.option = "bytes", .handler = &handle_bytes},
    {.option = "history", .handler = &handle_history},
    {.option = "current", .handler = &handle_current},
    {.option = "password", .handler = &handle_password},
    {.option = "delete", .handler = &handle_delete},
    {.option = "conc", .handler = &handle_conc},
};

static handle_arguments get_option_function(char *command) {
    for (int i = 0; i < TOTAL_OPTIONS; i++) {
        if (strcmp(command, options[i].option) == 0) {
            return options[i].handler;
        }
    }
    return NULL;
}

struct request *get_request_struct(int version, int argc, char *argv[]) {
    if (argc == 0) {
        error_and_exit(
            "missing arguments\nTry \"./client -h\" for more information.")
    }
    struct request *request = NULL;
    if (strcmp(argv[0], "-a") == 0) {
        argc--;
        argv++;
        request = calloc(1, sizeof(struct request));
        int arguments_consumed = handle_auth(argc, argv, request);
        argc -= arguments_consumed;
        argv += arguments_consumed;
    } else if (strcmp(argv[0], "-h") == 0 && argc == 1) {
        handle_help();
        exit(SUCCESS);
    } else {
        error_and_exit("first option must be authentication\nTry \"./client "
                       "-h\" for more information.")
    }

    if (argc == 0) {
        error_and_exit(
            "missing a command\nTry \"./client -h\" for more information.")
    }

    handle_arguments handler = get_option_function(argv[0]);

    if (handler == NULL) {
        free(request);
        error_and_exit(
            "not a valid command\nTry \"./client -h\" for more information.")
    }

    argc--;
    argv++;
    handler(argc, argv, request);

    request->version = version;
    request->req_id = 1;

    return request;
}

void get_request(char * buffer, struct request * request){
  strcpy(buffer, "popcorn\r\n");
  strcat(buffer, "version: ");
  strcat(buffer, "1");
  strcat(buffer, "\r\n");
  strcat(buffer, "auth: ");
  strcat(buffer, request->username);
  strcat(buffer, ":");
  strcat(buffer, request->password);
  strcat(buffer, "\r\n");
  strcat(buffer, "req-id: ");
  strcat(buffer, "1");
  strcat(buffer, "\r\n");
  strcat(buffer, "command: ");
  strcat(buffer, request->command);
  if (request->argument1[0] != '\0'){
    strcat(buffer, " ");
    strcat(buffer, request->argument1);
  }
  if (request->argument2[0] != '\0'){
    strcat(buffer, " ");
    strcat(buffer, request->argument2);
  }
  strcat(buffer, "\r\n");
}
