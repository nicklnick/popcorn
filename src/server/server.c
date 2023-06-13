#include "server_adt.h"
#include "utils.h"
#include "wrapper-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_CURRENT_CLIENTS 500

int main(int argc, char const *argv[]) {
    close(STDIN_FILENO);
    // close(STDOUT_FILENO);

    server_ptr server = init_server("../mail");
    int server_sock = get_server_socket();

    unsigned int childCount = 0;
    while (true) {
        if (childCount > MAX_CURRENT_CLIENTS) {
            continue;
        }

        int clientSocket = acceptConnection(server_sock);
        if (clientSocket < 0) {
            perror(ACCEPT_CONNECTION_ERROR);
            return 1;
        }

        pid_t pid = _fork();
        if (pid == 0) {
            close(server_sock);
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
