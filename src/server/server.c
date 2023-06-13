#include "server.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define PORT                110
#define MAX_CURRENT_CLIENTS 500

typedef enum {
    false,
    true
} bool;

int main(int argc, char const *argv[]) {
    close(STDIN_FILENO);
    // close(STDOUT_FILENO);

    int serverSock = setupServerSocket(PORT);
    if (serverSock < 0) {
        perror(SETUP_SERVER_SOCKET_ERROR);
        return 1;
    }

    unsigned int childCount = 0;
    while (true) {
        if (childCount > MAX_CURRENT_CLIENTS) {
            continue;
        }

        int clientSocket = acceptConnection(serverSock);
        if (clientSocket < 0) {
            perror(ACCEPT_CONNECTION_ERROR);
            return 1;
        }

        pid_t pid = _fork();
        if (pid == 0) {
            close(serverSock);
            handleConnection(clientSocket);
            exit(EXIT_SUCCESS);
        }

        close(clientSocket);
        childCount++;

        while (childCount) {
            pid = _waitpid((pid_t)-1, NULL, WNOHANG);
            if (pid == 0)
                break;
            else
                childCount--;
        }
    }
}
