#include "sm.h"
#include "../server/pop3-limits.h"
#include "../server/pop3-messages.h"
#include "../server/state-commands.h"
#include "../utils/staus-codes.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct state_machine {
    state current_state;
} state_machine;

typedef int (*StateFunc)(state_machine *self, session_ptr session, char *buff,
                         int nbytes);

int start(state_machine *self, session_ptr session, char *buff, int nbytes) {
    int len = 0;
    len = strlen(GREETING);
    strncpy(buff, GREETING, len);
    self->current_state = AUTHORIZATION;
    return len;
}

int auth(state_machine *self, session_ptr session, char *buff, int nbytes) {

    int len = 0;
    struct parser_event *event = get_session_event(session);

    if (strncmp(event->command, USER, nbytes) == 0) {
        len =
            auth_user_command(session, event->argument1, event->arg1_len, buff);

    } else if (strncmp(event->command, PASS, nbytes) == 0) {
        bool change_status = false;
        len = auth_pass_command(session,event->argument1,event->arg1_len,buff, &change_status);
        if(change_status) {
            init_client_dir_mails(session);
            self->current_state = TRANSACTION;
        }
    } else {
        len = strlen(ERR_COMMAND);
        strncpy(buff, ERR_COMMAND, len);
    }

    return len;
}

int transaction(state_machine *self, session_ptr session, char *buff,
                int nbytes) {

    int len = 0;
    struct parser_event *event = get_session_event(session);

    char response[RESPONSE_LEN] = {0};
    if (strcmp(event->command, QUIT) == 0) {
        len = strlen(OK_QUIT);
        strncpy(buff, OK_QUIT, len);
        self->current_state = END;

    } else if (strncmp(event->command, STAT, nbytes) == 0) {
        len = strlen(OK_STAT) + 1;
        strncpy(buff, OK_STAT, len);
        strcat(buff, " ");

        len += transaction_stat_command(session, event->argument1,
                                        event->arg1_len, response);
        strcat(buff, response);
        strcat(buff, "\r\n");
        len += 2;
    } else if (strncmp(event->command, LIST, nbytes) == 0) {
        len = transaction_list_command(session,event->argument1,event->arg1_len,buff);
    } else if (strncmp(event->command, RETR, nbytes) == 0) {

    } else if (strncmp(event->command, DELE, nbytes) == 0) {
        int status = transaction_dele_command(session, event->argument1,
                                              event->arg1_len, response);
        if (status == ERROR) {
            len = strlen(response);
            strncpy(buff, response, len);
            return len;
        }
        len = strlen(OK_DELE);
        strncpy(buff, OK_DELE, len);
    } else if (strncmp(event->command, NOOP, nbytes) == 0) {
        len = strlen(OK_NOOP);
        strncpy(buff, OK_NOOP, len);
    } else if (strncmp(event->command, RSET, nbytes) == 0) {
        transaction_rset_command(session, event->argument1, event->arg1_len,
                                 response);
        len = strlen(OK_RSET);
        strncpy(buff, OK_RSET, len);
    } else {
        len = strlen(ERR_COMMAND);
        strncpy(buff, ERR_COMMAND, len);
        self->current_state = TRANSACTION;
    }

    return len;
}

int end(state_machine *self, session_ptr session, char *buff, int nbytes) {
    printf("[END] +OK\n");
    self->current_state = END;

    return SUCCESS;
}

StateFunc func_table[4] = {
    &start,       // START
    &auth,        // AUTHORIZATION
    &transaction, // TRANSACTION
    &end,         // END
};

int dispatch(state_machine *self, session_ptr session, char *buff, int nbytes) {
    return (*func_table[self->current_state])(self, session, buff, nbytes);
}

state_machine *new_state_machine() {
    state_machine *state_machine = malloc(sizeof(state_machine));
    state_machine->current_state = START;

    return state_machine;
}

state get_current_state(state_machine_ptr state_machine) {
    return state_machine->current_state;
}

void free_state_machine(state_machine *state_machine) {
    free(state_machine);
}
