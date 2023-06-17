#include "session.h"
#include "../buffer/buffer.h"
#include "../parser/command_parser.h"
#include "../server/wrapper-functions.h"
#include "../sm/sm.h"
#include "../utils/file-utils.h"
#include "../utils/general-utils.h"
#include "../utils/staus-codes.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define BUFFER_SIZE 256

struct client_dir {
    DIR *dir_pt;

    int *mails; // has 1 if it is marked to be deleted
    int total_mails;
};

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

    struct client_dir *client_dir;
    struct fd_handler *client_fd_handler;
    int wbytes;

} client_session;

client_session *new_client_session(int client_socket) {
    client_session *session = malloc(sizeof(client_session));
    session->socket = client_socket;
    memset(session->username, 0, BUFFER_SIZE);
    session->state_machine = new_state_machine();
    session->command_parser = command_parser_init();
    session->event = get_command_parser_event(session->command_parser);
    session->client_fd_handler = calloc(1, sizeof(fd_handler));
    session->client_fd_handler->handle_read = session_read;
    session->client_fd_handler->handle_write = session_send_response;
    session->wbytes = 0;
    buffer_init(&session->rbuffer, BUFFER_SIZE,
                (uint8_t *)session->rbuffer_data);
    buffer_init(&session->wbuffer, BUFFER_SIZE,
                (uint8_t *)session->wbuffer_data);
    session->client_dir = _calloc(1, sizeof(struct client_dir));

    return session;
}

void session_read(struct selector_key *key) {

    session_ptr session = (session_ptr)key->data;

    if (session->event->type != MAY_VALID) {
        command_parser_reset(session->command_parser);
    }

    if (session->event->type == MAY_VALID) {
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

    if (session->event->type != MAY_VALID) {
        session->wbytes = session_process(session);
        selector_set_interest_key(key, OP_WRITE);
    }
}

int session_process(session_ptr session) {
    size_t wsize = 0;
    char *wbuffer = (char *)buffer_write_ptr(&session->wbuffer, &wsize);
    int wbytes = dispatch(session->state_machine, session, wbuffer, (int)wsize);
    buffer_write_adv(&session->wbuffer, wbytes);
    return wbytes;
}

void session_send_response(struct selector_key *key) {

    session_ptr session = key->data;

    if (get_current_state(session->state_machine) == START)
        session->wbytes = session_process(session);

    if (session->wbytes == 0 && buffer_can_read(&session->rbuffer)) {
        session_read(key);
        return;
    }

    int wbytes = session->wbytes;

    if (wbytes > 0) {
        size_t rsize = 0;
        char *rbuffer = (char *)buffer_read_ptr(&session->wbuffer, &rsize);
        int bytes_sent = (int)_send(session->socket, rbuffer, rsize, 0);
        buffer_read_adv(&session->wbuffer, bytes_sent);
        wbytes -= bytes_sent;
        session->wbytes = wbytes;
    }

    if (session->wbytes == 0 && !buffer_can_read(&session->rbuffer)) {
        selector_set_interest_key(key, OP_READ);
        return;
    }
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

void set_client_dir_pt(session_ptr session, DIR *dir) {
    session->client_dir->dir_pt = dir;
}

DIR *get_client_dir_pt(session_ptr session) {
    rewinddir(session->client_dir->dir_pt);
    return session->client_dir->dir_pt;
}

void init_client_dir_mails(session_ptr session) {
    int file_count = get_file_count(session->client_dir->dir_pt);
    session->client_dir->mails = (int *)_calloc(file_count, sizeof(int));
    session->client_dir->total_mails = file_count;
}

static char is_marked(session_ptr session, int mail_num) {
    return session->client_dir->mails[mail_num - 1] == true;
}

int mark_to_delete(session_ptr session, int mail_num) {
    if (is_marked(session, mail_num) ||
        !IS_BETWEEN(mail_num, 0, session->client_dir->total_mails)) {
        return ERROR;
    }

    session->client_dir->mails[mail_num - 1] = true;
    return SUCCESS;
}

void unmark_mails(session_ptr session) {
    memset(session->client_dir->mails, 0,
           sizeof(session->client_dir->mails[0]) *
               session->client_dir->total_mails);
}

int *get_client_dir_mails(session_ptr session) {
    return session->client_dir->mails;
}

fd_handler *get_fd_handler(session_ptr session) {
    return session->client_fd_handler;
}

void close_client_session(session_ptr session) {
    free_state_machine(session->state_machine);
    command_parser_destroy(session->command_parser);
    // FIXME: Tirar error porque el comand_parser retorna su propio event
    // entonces ya fue liberado. free(session->event);
    free(session->client_fd_handler);
    free(session);
}
