#include "session.h"
#include "../buffer/buffer.h"
#include "../parser/command_parser.h"
#include "../pop3-limits.h"
#include "../wrapper-functions.h"
#include "../sm/sm.h"
#include "../utils/file-utils.h"
#include "../utils/general-utils.h"
#include "../utils/stack_adt.h"
#include "../utils/staus-codes.h"
#include "../utils/logger.h"
#include "../server_adt.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>

struct client_dir {
    DIR *dir_pt;
    int pt_index;

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

    stack_adt action_stack;

    struct retr_state * retr_state;

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
    session->client_fd_handler->handle_close = close_client_fd_handler;
    session->wbytes = 0;
    session->action_stack = new_stack_adt();
    push_action_state(session,READ);
    push_action_state(session,PROCESS);
    buffer_init(&session->rbuffer, BUFFER_SIZE,
                (uint8_t *)session->rbuffer_data);
    buffer_init(&session->wbuffer, BUFFER_SIZE,
                (uint8_t *)session->wbuffer_data);
    session->client_dir = _calloc(1, sizeof(struct client_dir));
    session->client_dir->pt_index = 1;

    session->retr_state = _calloc(1,sizeof (struct retr_state));

    return session;
}

void session_read(struct selector_key *key) {

    session_ptr session = (session_ptr)key->data;
    action_state current = peek_action_state(session);

    if (session->event->type != MAY_VALID) {
        command_parser_reset(session->command_parser);
    }

    if (current == READ) {
        size_t wsize = 0;
        char *wbuffer = (char *)buffer_write_ptr(&session->rbuffer, &wsize);
        ssize_t bytes_recv = _recv(session->socket, wbuffer, wsize, 0);
        if(bytes_recv == 0){
            selector_unregister_fd(key->s,key->fd);
            return ;
        }
        buffer_write_adv(&session->rbuffer, bytes_recv);
    }

    size_t rsize = 0;
    size_t nread = 0; // bytes that were read

    char *rbuffer = (char *)buffer_read_ptr(&session->rbuffer, &rsize);
    session->event = get_command(session->event, session->command_parser,
                                 rbuffer, rsize, &nread);
    buffer_read_adv(&session->rbuffer, (ssize_t)nread);

    if (session->event->type != MAY_VALID) {

        if (buffer_can_read(&session->rbuffer)) {
            push_action_state(session, READING);
        }
        else {
            if(current == READING)
                pop_action_state(session);
        }

        push_action_state(session,PROCESS);
        session->wbytes = session_process(session);
        selector_set_interest_key(key, OP_WRITE);
    }
}

int session_process(session_ptr session) {
    size_t wsize = 0;
    char *wbuffer = (char *)buffer_write_ptr(&session->wbuffer, &wsize);
    int wbytes = dispatch(session->state_machine, session, wbuffer, (int)wsize);
    buffer_write_adv(&session->wbuffer, wbytes);

    push_action_state(session, WRITE);

    return wbytes;
}

void session_send_response(struct selector_key *key) {

    session_ptr session = key->data;
    action_state current = peek_action_state(session);

    if (current == PROCESS || current == PROCESSING) {
        session->wbytes = session_process(session);
        return ;
    }

    if (current == READING) {
        session_read(key);
        return;
    }

    int wbytes = session->wbytes;

    if (wbytes > 0) {
        size_t rsize = 0;
        char *rbuffer = (char *)buffer_read_ptr(&session->wbuffer, &rsize);
        errno = 0;
        int bytes_sent = (int)send(session->socket, rbuffer, rsize, MSG_NOSIGNAL);
        //Client close the connection
        if(bytes_sent == -1 && errno == EPIPE){
            selector_unregister_fd(key->s,key->fd);
            return;
        }
        buffer_read_adv(&session->wbuffer, bytes_sent);
        wbytes -= bytes_sent;
        session->wbytes = wbytes;
    }

    if (session->wbytes != 0) {
        if (current == WRITE) {
            pop_action_state(session);
        }
        if (current != WRITING)
            push_action_state(session, WRITING);
        return;
    }

    if (current == WRITING || current == WRITE) {
        pop_action_state(session);
        current = peek_action_state(session);
    }

    if (current == READ) {
        selector_set_interest_key(key, OP_READ);
        return;
    }

    if(get_session_state(session) == END){
        selector_unregister_fd(key->s,key->fd);
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
    return session->client_dir->dir_pt;
}

int get_client_dir_pt_index(session_ptr session){
    return session->client_dir->pt_index;
}

void set_client_dir_pt_index(session_ptr session, int index){
    session->client_dir->pt_index = index;
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
    if (is_marked(session, mail_num))
        return STATUS_ERROR;

    session->client_dir->mails[mail_num - 1] = true;
    return STATUS_SUCCESS;
}

void unmark_mails(session_ptr session) {
    memset(session->client_dir->mails, 0,
           sizeof(session->client_dir->mails[0]) *
               session->client_dir->total_mails);
}

int *get_client_dir_mails(session_ptr session) {
    return session->client_dir->mails;
}

int get_client_total_mails(session_ptr session){
    return session->client_dir->total_mails;
}

fd_handler *get_fd_handler(session_ptr session) {
    return session->client_fd_handler;
}

struct retr_state * get_session_retr_state(session_ptr session){
    return session->retr_state;
}

void close_client_session(session_ptr session) {
    logv(INFO, "Closing session for client with socket [%d]...", session->socket)

    close(session->socket);
    remove_client(session);
    free_state_machine(session->state_machine);
    command_parser_destroy(session->command_parser);
    // FIXME: Tirar error porque el comand_parser retorna su propio event
    // entonces ya fue liberado. free(session->event);
    free_stack_adt(session->action_stack);

    struct user_dir * user_d = get_user_dir(session->username, strlen(session->username));
    if(user_d != NULL)
        user_d->is_open = false;

    closedir(session->client_dir->dir_pt);
    free(session->client_dir->mails);
    free(session->client_dir);
    free(session->client_fd_handler);
    free(session->retr_state);
    free(session);

    log(INFO, "Session closed")
}

void close_client_fd_handler (struct selector_key *key){
    if(key->data == NULL)
        return ;
    close_client_session(key->data);
}

action_state pop_action_state(session_ptr session) {
    stack_data_t action_data;
    int result = pop(session->action_stack, &action_data);
    if (result == -1)
        return -1;
    return action_data.action;
}

action_state peek_action_state(session_ptr session) {
    stack_data_t action_data;
    int result = peek(session->action_stack, &action_data);
    if (result == -1)
        return -1;
    return action_data.action;
}

void push_action_state(session_ptr session, action_state action) {
    stack_data_t action_data;
    action_data.action = action;
    push(session->action_stack, action_data);
}

