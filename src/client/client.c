#include "request.h"
#include "response_parser.h"
#include <errno.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define BUFFER_SIZE 1024
#define POPCORN_PORT 2882

int main(int argc, char *argv[]) {
  
  struct request * request = get_request_struct(1, argc-1, argv+1);

  char request_buffer[BUFFER_SIZE]={0};
  char response_buffer[BUFFER_SIZE]={0};

  get_request(request_buffer, request);

  ssize_t request_buffer_len = strlen(request_buffer);

  errno = 0;
  int client_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (client_socket < 0){
    free(request);
    error_and_exit("socket() failed: %s", strerror(errno))
  }

  struct sockaddr_in address; 
  unsigned int address_length = sizeof(address);

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = htonl(INADDR_ANY);
  address.sin_port = htons(POPCORN_PORT);

  struct timeval tv;
  tv.tv_sec = 5;
  tv.tv_usec = 0;
  if (setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      free(request);
      close(client_socket);
      error_and_exit("setsockopt error: %s", strerror(errno));
  }

  ssize_t num_bytes = sendto(client_socket, request_buffer, 
    request_buffer_len, 0, (struct sockaddr *) &address, address_length);
  if (num_bytes < 0) {
    free(request);
    close(client_socket);
    error_and_exit("sendto() failed: %s", strerror(errno))
  } else if (num_bytes != request_buffer_len) {
    free(request);
    close(client_socket);
    error_and_exit("sendto() error, sent unexpected number of bytes")
  }       

  num_bytes = recvfrom(client_socket, response_buffer, 
    sizeof(response_buffer), 0, (struct sockaddr *) &address, &address_length);
  if (num_bytes < 0) {
    free(request);
    close(client_socket);
    error_and_exit("recvfrom() failed: %s", strerror(errno))
  } else {
    response_buffer[num_bytes] = '\0';     
  }

  struct popcorn_response * response = calloc(1, sizeof(struct popcorn_response));
  parse_response(response_buffer, response);

  switch(response->status_code){
    case 20:{
      printf("response: OK\n");
      if (response->response_value[0] != '\0')
        printf("value: %s\n", response->response_value);
      break;
    }
    case 40: {
      error("response: ERROR - Client error")
      break;
    }
    case 41: {
      error("response: ERROR - Bad credentials");
      break;
    }
    case 42: {
      error("response: ERROR - User does not exists")
      break;
    }
    case 43: {
      error("response: ERROR - User is currently logged in")
      break;
    }
    case 49: {
      error("response: ERROR - Version not supported")
      break;
    }
    case 50: {
      error("response: ERROR - Server error")
      break;
    }
    default: {
      error("response: ERROR - Unknown status code")
      break;
    }
  }
  
  free(request);
  free(response);
  close(client_socket);
  return 0;
  
}
