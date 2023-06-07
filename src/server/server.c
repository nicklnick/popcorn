#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "server.h"
#include "utils.h"

#define PORT 110
#define MAX_CURRENT_CLIENTS 500

typedef enum {false,true} bool;


int main(int argc, char const *argv[]){

    close(0);
    close(1);

    int serverSock = setupServerSocket(PORT);
    if(serverSock < 0){
        perror(SETUP_SERVER_SOCKET_ERROR);
        return 1;
    }

    unsigned int childCount = 0;

    while (true){

        if(childCount > MAX_CURRENT_CLIENTS){
            continue;
        }

        int clntSock = acceptConnection(serverSock);
        if(clntSock < 0){
            perror(ACCEPT_CONNECTION_ERROR);
            return 1;
        }

        pid_t pid = fork();
        if(pid < 0){
            perror("fork()");
        }
        else if (pid == 0){
            close(serverSock);
            exit(0);
        }

        close(clntSock);
        childCount++;

        while(childCount){
            pid = waitpid((pid_t) -1, NULL, WNOHANG);
            if(pid < 0){
                perror("waitpid()");
            }
            else if( pid == 0){
                break;
            }
            else
                childCount--;
        }
    }
    
}



