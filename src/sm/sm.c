#include "sm.h"
#include "../server/pop3-messages.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct state_machine {
  state current_state;
} state_machine;

typedef int (*StateFunc)(state_machine *self, struct parser_event *event, char * buff, int nbytes);

int start(state_machine *self, struct parser_event *event, char * buff, int nbytes) {
  printf("+OK READY\n");
  printf("[AUTHORIZATION] ENTRY\n\n");
  self->current_state = AUTHORIZATION;
}

int auth(state_machine *self, struct parser_event *event, char * buff, int nbytes) {

    int len;

  if (strncmp(event->command, USER, nbytes) == 0) {
      len = strlen(OK_USER);
      strncpy(buff, OK_USER, len);
  } else if (strncmp(event->command, PASS, nbytes) == 0) {
      len = strlen(OK_PASS);
      strncpy(buff, OK_PASS, len);
    self->current_state = TRANSACTION;
  } else {
      len = strlen(ERR_COMMAND);
      strncpy(buff, ERR_COMMAND, len);
    self->current_state = AUTHORIZATION;
  }

  return len;
}

int transaction(state_machine *self, struct parser_event *event, char * buff, int nbytes) {

  if (strcmp(event->command, "QUIT") == 0) {
    printf("[TRANSACTION] +OK QUIT\n");
    self->current_state = END;
    printf("[END] ENTRY\n\n");
  } else {
    printf("[TRANSACTION] -OK ERROR\n");
    self->current_state = TRANSACTION;
  }

}

int end(state_machine *self, struct parser_event *event, char * buff, int nbytes) {
  printf("[END] +OK\n");
  self->current_state = END;
}

StateFunc func_table[4] = {
    &start,       // START
    &auth,        // AUTHORIZATION
    &transaction, // TRANSACTION
    &end,         // END
};

int dispatch(state_machine *self, struct parser_event *event, char * buff, int nbytes) {
  return (*func_table[self->current_state])(self, event, buff, nbytes);
}

state_machine *new_state_machine() {
  state_machine *state_machine = malloc(sizeof(state_machine));
  state_machine->current_state = AUTHORIZATION;

  return state_machine;
}

state get_current_state(state_machine_ptr state_machine){
    return  state_machine->current_state;
}

void free_state_machine(state_machine *state_machine) { free(state_machine); }
