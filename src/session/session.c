#include "session.h"
#include "../buffer/buffer.h"
#include "../parser/command_parser.h"
#include "../sm/sm.h"
#include "wrapper-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 256

typedef struct client_session {
    int socket;
    char username[BUFFER_SIZE];
    buffer rbuffer;
    char rbuffer_data[BUFFER_SIZE];
    buffer wbuffer;
    char wbuffer_data[BUFFER_SIZE];
    state_machine_ptr state_machine;
    struct parser *command_parser;
    struct parser_event *event;
} client_session;

client_session *new_client_session(int client_socket) {
    client_session *session = malloc(sizeof(client_session));
    session->socket = client_socket;
    memset(session->username, 0, BUFFER_SIZE);
    session->state_machine = new_state_machine();
    session->command_parser = command_parser_init();
    session->event = _malloc(sizeof(struct parser_event));
    buffer_init(&session->rbuffer, BUFFER_SIZE,
                (uint8_t *)session->rbuffer_data);
    buffer_init(&session->wbuffer, BUFFER_SIZE,
                (uint8_t *)session->wbuffer_data);

    return session;
}

struct parser_event *session_read(session_ptr session) {

    // FIXME: Tidy up
    session->event->cmd_len = 0;
    session->event->arg1_len = 0;
    session->event->arg2_len = 0;
    session->event = _malloc(sizeof(struct parser_event));

    while (session->event->type == MAY_VALID) {
        if (!buffer_can_read(&session->rbuffer)) {
            size_t wsize = 0;
            char *wbuffer = (char *)buffer_write_ptr(&session->rbuffer, &wsize);
            ssize_t bytes_recv = _recv(session->socket, wbuffer, wsize, 0);

            buffer_write_adv(&session->rbuffer, bytes_recv);
        }
        size_t rsize = 0;
        size_t nread = 0; // bytes that were read
        char *rbuffer = (char *)buffer_read_ptr(&session->rbuffer, &rsize);
        session->event = get_command(session->event, session->command_parser,
                                     rbuffer, rsize, &nread);

        buffer_read_adv(&session->rbuffer, (ssize_t)nread);
    }
    parser_reset(session->command_parser);

    return session->event;
}

int session_process(session_ptr session) {
    size_t wsize = 0;
    char *wbuffer = (char *)buffer_write_ptr(&session->wbuffer, &wsize);
    int wbytes = dispatch(session->state_machine, session, wbuffer, (int)wsize);
    buffer_write_adv(&session->wbuffer, wbytes);
    return wbytes;
}

void session_send_response(session_ptr session, int wbytes) {
    while (wbytes > 0) {
        size_t rsize = 0;
        char *rbuffer = (char *)buffer_read_ptr(&session->wbuffer, &rsize);
        int bytes_sent = (int)_send(session->socket, rbuffer, rsize, 0);
        buffer_read_adv(&session->wbuffer, bytes_sent);
        wbytes -= bytes_sent;
    }
}

int session_write_response(session_ptr session, char *response,
                           int response_len) {

    size_t wsize = 0;
    char *wbuffer = (char *)buffer_write_ptr(&session->wbuffer, &wsize);

    int w_bytes = (response_len <= wsize) ? response_len : (int)wsize;
    strncpy(wbuffer, response, w_bytes);
    buffer_write_adv(&session->wbuffer, w_bytes);
    return w_bytes;
}

state get_session_state(session_ptr session) {
    return get_current_state(session->state_machine);
}

int get_username(session_ptr session, char *username) {
    if (session->username[0] == '\0')
        return -1;
    int len = (int)strlen(session->username);
    strncpy(username, session->username, len);
    return len;
}

void set_username(session_ptr session, char *username, int len) {
    strncpy(session->username, username, len);
}

struct parser_event *get_session_event(session_ptr session) {
    return session->event;
}