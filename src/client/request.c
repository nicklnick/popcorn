#include "request.h"

struct request{
  int version; 
  char username[NAME_SIZE];
  char password[PASSWORD_SIZE];
  int req_id;
  char * command;
  char * argument1;
  char * argument2;
} request;

typedef int(*handle_arguments)(int argc, char *argv[], struct request * request);

struct option{
  char option[OPTION_SIZE];
  handle_arguments handler;
} option;

static void handle_help(){
  printf("POPCORN protocol client\n");
  printf("Usage: ./popcorn_client -a USER:PASSWORD COMMAND [ARGUMENT]...\n\n");
  printf("Commands:\n");
  printf("  current\n");
  printf("  history\n");
  printf("  bytes\n");
  printf("  password USER PASSWORD\n");
  printf("  delete   USER\n");
}

static int handle_auth(int argc, char *argv[], struct request * request){
  if (argc < 1){
    error_and_exit("-a: missing username and password")
  }
  char * username=strtok(argv[0], ":");
  if (username == NULL){
      error_and_exit("-a: format for authentication is user:password")
  }
  char * password = strtok(NULL, ":");
  if (password == NULL){
      error_and_exit("-a: no password provided for username \"%s\"", username)
  }
  if (strlen(username) > NAME_SIZE){
    error_and_exit("-a: username length is too long")
  }
  if (strlen(password) > PASSWORD_SIZE){
    error_and_exit("-a: password size is too long")
  }
  strcpy(request->username, username);
  strcpy(request->password, password);
  return 1;
}

static int handle_current(int argc, char ** argv, struct request * request){
  request->command= "current";
  return 0;
}

struct option options[32] = {
  {.option="current", .handler=handle_current}
};

struct request * get_request_struct(int version, int argc, char * argv[]){
    if (argc == 0){
      error_and_exit("missing arguments\nTry \"./popcorn_client -h\" for more information.")
    }
    struct request * request = NULL;
    if (strcmp(argv[0], "-a") == 0){
      argc--;
      argv++;
      request = calloc(1, sizeof(struct request));
      int arguments_consumed = handle_auth(argc, argv, request);
      argc -= arguments_consumed;
      argv += arguments_consumed;
    }
    else if (strcmp(argv[0], "-h") == 0 && argc == 1){
        handle_help();
        exit(SUCCESS);
    }
    else{
      error_and_exit("first option must be authentication\nTry \"./popcorn_client -h\" for more information.")
    }

    if (argc == 0){
      error_and_exit("missing a command\nTry \"./popcorn_client -h\" for more information.")
    }
    
    bool found_command = false;
    for (int i=0; i < TOTAL_OPTIONS; i++){
      if (strcmp(argv[0], options[i].option) == 0){
        found_command=true;
        argc--;
        argv++;
        int arguments_consumed = options[i].handler(argc, argv, request);
        argc -= arguments_consumed;
        argv += arguments_consumed;
        if (argc != 0){
          free(request);
          error_and_exit("too many arguments\nTry \"./popcorn_client -h\" for more information.")
        }
      }
    }

    if (!found_command){
      free(request);
      error_and_exit("not a valid command\nTry \"./popcorn_client -h\" for more information.")
    }

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
  if (request->argument1 != NULL){
    strcat(buffer, " ");
    strcat(buffer, request->argument1);
  }
  if (request->argument2 != NULL){
    strcat(buffer, " ");
    strcat(buffer, request->argument2);
  }
  strcat(buffer, "\r\n");
}