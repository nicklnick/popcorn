#include "sm.h"
#include "../pop3-limits.h"
#include "../pop3-messages.h"
#include "../state-commands.h"
#include "../utils/staus-codes.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void string_to_upper(char* str) {
    if (str == NULL) {
        return;
    }
    for (int i = 0; str[i] != '\0'; i++) {
        str[i] = toupper((unsigned char)str[i]);
    }
}

typedef struct state_machine {
    state current_state;
} state_machine;

typedef int (*StateFunc)(state_machine *self, session_ptr session, char *buff,
                         int nbytes);

int start(state_machine *self, session_ptr session, char *buff, int nbytes) {

    pop_action_state(session);

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
    } else if (strncmp(event->command,QUIT,nbytes) == 0){
        len = strlen(OK_QUIT);
        strncpy(buff, OK_QUIT, len);
        self->current_state = END;
    }else if (strncmp(event->command,CAPA,nbytes) == 0) {
        pop_action_state(session);
        len = strlen(CAPA_AUTH);
        strncpy(buff, CAPA_AUTH, len);
    }
    else {
        pop_action_state(session);
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
    if (strncmp(event->command, QUIT,nbytes) == 0) {
        transaction_quit_command(session);
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
        len = transaction_list_command(session,event->argument1,event->arg1_len,buff,nbytes);
    } else if (strncmp(event->command, RETR, nbytes) == 0) {
        len = transaction_retr_command(session,event->argument1,event->arg1_len,buff,nbytes);
    } else if (strncmp(event->command, DELE, nbytes) == 0) {
        int status = transaction_dele_command(session, event->argument1,
                                              event->arg1_len, response);
        if (status == STATUS_ERROR) {
            len = strlen(response);
            strncpy(buff, response, len);
            return len;
        }
        len = strlen(OK_DELE);
        strncpy(buff, OK_DELE, len);
    } else if (strncmp(event->command, NOOP, nbytes) == 0) {
        pop_action_state(session);
        len = strlen(OK_NOOP);
        strncpy(buff, OK_NOOP, len);
    } else if (strncmp(event->command, RSET, nbytes) == 0) {
        transaction_rset_command(session, event->argument1, event->arg1_len,
                                 response);
        len = strlen(OK_RSET);
        strncpy(buff, OK_RSET, len);
    }else if (strncmp(event->command,CAPA,nbytes) == 0) {
        pop_action_state(session);
        len = strlen(CAPA_TRANSACTION);
        strncpy(buff, CAPA_TRANSACTION,len);
    }
    else {
        pop_action_state(session);
        len = strlen(ERR_COMMAND);
        strncpy(buff, ERR_COMMAND, len);
        self->current_state = TRANSACTION;
    }

    return len;
}

int end(state_machine *self, session_ptr session, char *buff, int nbytes) {
    printf("[END] +OK\n");
    self->current_state = END;

    return STATUS_SUCCESS;
}

StateFunc func_table[4] = {
    &start,       // START
    &auth,        // AUTHORIZATION
    &transaction, // TRANSACTION
    &end,         // END
};

int dispatch(state_machine *self, session_ptr session, char *buff, int nbytes) {
    struct parser_event *event = get_session_event(session);
    string_to_upper(event->command);
    return (*func_table[self->current_state])(self, session, buff, nbytes);
}

state_machine *new_state_machine(void) {
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
