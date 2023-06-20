#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>

#define BUFFER_SIZE 1024

#define POPCORN_PORT "2882"

#define NAME_SIZE 16
#define PASSWORD_SIZE 16
#define OPTION_SIZE 32
#define TOTAL_OPTIONS 1

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

static int handle_auth(int argc, char *argv[], struct request * request){
  char * username=strtok(argv[0], ":");
  char * password = strtok(NULL, ":");
  if (strlen(username) > NAME_SIZE){
    printf("client: username length is too long");
    exit(1);
  }
  if (strlen(password) > PASSWORD_SIZE){
    printf("client: password size is too long");
    exit(1);
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
    // -a user:pass current 
    if (argc < 3){
      printf("Missing arguments\n");
      printf("Usage: ./client -a user:pass command\n");
      exit(1);
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
    else{
      printf("client: requires authentication first\n");
      printf("client: usage: ./client -a user:pass command\n");
      exit(1);
    }

    for (int i=0; i < TOTAL_OPTIONS; i++){
      if (strcmp(argv[0], options[i].option) == 0){
        argc--;
        argv++;
        int arguments_consumed = options[i].handler(argc, argv, request);
        argc -= arguments_consumed;
        argv += arguments_consumed;
        if (argc != 0){
          printf("argc: %d\n", argc);
          printf("argv=: %s\n", argv[0]);
          printf("client: too many arguments\n");
          printf("client: usage: ./client -a user:pass command\n");
          free(request);
          exit(1);
        }
        else{
          break;
        }
      }
    }

    request->version = version;
    request->req_id = 1;

    return request;
}

static void get_request(char * buffer, struct request * request){
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

int main(int argc, char *argv[]) {
  
  struct request * request = get_request_struct(1, argc-1, argv+1);

  char request_buffer[BUFFER_SIZE]={0};
  char response_buffer[BUFFER_SIZE]={0};

  get_request(request_buffer, request);
  printf("%s", request_buffer); 

  return 1;
  ssize_t request_buffer_len = strlen(request_buffer);

  char *serv_port = POPCORN_PORT;

  errno = 0;
  int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (client_socket < 0)
    return 1;
    //log(FATAL, "socket() failed: %s", strerror(errno));

  struct sockaddr_in address; 
  unsigned int address_length = sizeof(address);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(serv_port);

  // Establecemos un timeout de 5 segundos para la respuesta
  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      return 1;
      //log(ERROR, "setsockopt error: %s", strerror(errno))
  }

  // Enviamos el string
  ssize_t num_bytes = sendto(client_socket, request_buffer, 
    request_buffer_len, 0, (struct sockaddr *) &address, address_length);
  if (num_bytes < 0) {
    return 1;
    //log(FATAL, "sendto() failed: %s", strerror(errno))
  } else if (num_bytes != request_buffer_len) {
    return 1;
    //log(FATAL, "sendto() error, sent unexpected number of bytes");
  }       

  num_bytes = recvfrom(client_socket, response_buffer, 
    sizeof(response_buffer), 0, (struct sockaddr *) &address, &address);
  if (num_bytes < 0) {
    printf("recvfrom() failed: %s", strerror(errno));
    return 1;
    //log(ERROR, "recvfrom() failed: %s", strerror(errno))
  } else {
    response_buffer[num_bytes] = '\0';     
    printf("Received: %s\n", response_buffer); 
  }
  
  free(request);
  close(client_socket);
  return 0;
  
}
