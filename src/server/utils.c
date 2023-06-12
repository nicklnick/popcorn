#include "utils.h"
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "wrapper-functions.h"
#include "../parser/command_parser.h"
#include "../sm/sm.h"



int setupServerSocket(int port) {
    // IPv4 address
    // INADDR_ANY  (0.0.0.0)  means any address for binding
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);
    // TODO: Accept both ipv4 and ipv6 connections

    int server = _socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    _setsockopt(server, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int));

    _bind(server, (struct sockaddr *)&addr, sizeof(addr));
    _listen(server, 20);

    return 0;
}

int acceptConnection(int serverSock) {
    struct sockaddr_storage clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);

    return _accept(serverSock, (struct sockaddr *)&clientAddr, &clientAddrLen);
}

int handleConnection(int clientSocket) {


    struct parser *command_parser = command_parser_init();
    struct parser_event *event;

    state_machine_ptr state_machine = new_state_machine();

    struct rw_buffer rbuffer={{0},0,0};
    char wbuffer[BUFSIZE] = {0};



    while(get_current_state(state_machine) == AUTHORIZATION) {

        int r_index_cpy = 0;

        event = malloc(sizeof(struct parser_event));

        while(event->type == MAY_VALID){
            if(rbuffer.r_index == rbuffer.w_index){
                event->bytes_recv = _recv(clientSocket, rbuffer.buffer + rbuffer.w_index, BUFSIZE-rbuffer.w_index, 0);
                rbuffer.w_index+=event->bytes_recv;
            }
            r_index_cpy = rbuffer.r_index;
            event = get_command(event, command_parser, &rbuffer, rbuffer.w_index-rbuffer.r_index);
        }

        parser_reset(command_parser);

        int w_bytes = dispatch(state_machine,event,wbuffer,rbuffer.w_index - r_index_cpy);

        ssize_t bytes_sent = 0;
        while (w_bytes > 0) {
            bytes_sent = _send(clientSocket, wbuffer + bytes_sent, w_bytes, 0);
            w_bytes = w_bytes - bytes_sent;
        }
    }

    free(event);
    parser_destroy(command_parser);
    close(clientSocket);

    return 0;
}