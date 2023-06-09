#include "sm.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct state_machine {
  state current_state;
} state_machine;

typedef void (*StateFunc)(state_machine *self, struct parser *parser);

StateFunc start(state_machine *self, struct parser *parser) {
  printf("+OK READY\n");
  printf("[AUTHORIZATION] ENTRY\n\n");
  self->current_state = AUTHORIZATION;
}

StateFunc auth(state_machine *self, struct parser *parser) {
  struct parser_event *event = malloc(sizeof(struct parser_event));
  event = get_command(event, parser);

  if (strcmp(event->command, "USER") == 0) {
    printf("[AUTHORIZATION] +OK USER\n");
  } else if (strcmp(event->command, "PASS") == 0) {
    printf("[AUTHORIZATION] +OK PASS\n");
    self->current_state = TRANSACTION;
    printf("[TRANSACTION] ENTRY\n\n");
  } else {
    printf("[AUTHORIZATION] -ERR \n");
    self->current_state = AUTHORIZATION;
  } 

  parser_reset(parser);
}

StateFunc transaction(state_machine *self, struct parser *parser) {
  struct parser_event *event = malloc(sizeof(struct parser_event));
  event = get_command(event, parser); 

  if (strcmp(event->command, "QUIT") == 0) {
    printf("[TRANSACTION] +OK QUIT\n");
    self->current_state = END;
    printf("[END] ENTRY\n\n");
  } else {
    printf("[TRANSACTION] -OK ERROR\n");
    self->current_state = TRANSACTION;
  }

  parser_reset(parser);
  free(event);
}

StateFunc end(state_machine *self, struct parser *parser) {
  struct parser_event *event = malloc(sizeof(struct parser_event));
  event = get_command(event, parser);

  printf("[END] +OK\n");
  self->current_state = END;
  parser_reset(parser);
}

StateFunc func_table[4] = {
    &start,       // START
    &auth,        // AUTHORIZATION
    &transaction, // TRANSACTION
    &end,         // END
};

void dispatch(state_machine *self, struct parser *parser) {
  (*func_table[self->current_state])(self, parser);
}

state_machine *new_state_machine() {
  state_machine *state_machine = malloc(sizeof(state_machine));
  state_machine->current_state = START;

  return state_machine;
}

void free_state_machine(state_machine *state_machine) { free(state_machine); }