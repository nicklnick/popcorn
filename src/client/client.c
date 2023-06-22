#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include "request.h"

#define BUFFER_SIZE 1024
#define POPCORN_PORT 2882

int main(int argc, char *argv[]) {
  
  struct request * request = get_request_struct(1, argc-1, argv+1);

  char request_buffer[BUFFER_SIZE]={0};
  char response_buffer[BUFFER_SIZE]={0};

  get_request(request_buffer, request);
  printf("%s", request_buffer); 

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
    printf("Received: %s\n", response_buffer); 
  }

  int status = 0;
  char * value= NULL;
  switch(status){
    case 20:{
      printf("%s\n", value);
      break;
    }
    case 40: {
      error("Client error")
      break;
    }
    case 50: {
      error("Server error")
      break;
    }
    case 41: {
      error("Bad credentials");
      break;
    }
    case 42: {
      error("User does not exists")
      break;
    }
    default: {
      error("Unknown status code")
      break;
    }
  }
  
  free(request);
  close(client_socket);
  return 0;
  
}
