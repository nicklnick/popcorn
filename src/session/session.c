#include "session.h"
#include "../parser/command_parser.h"
#include "../sm/sm.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct user_session {
  long id;
  char *username;
  bool is_auth;
  state_machine_ptr state_machine;
  connection connection; 
} user_session;

static long id = 0;

user_session *new_user_session(connection connection) {
  user_session *session = malloc(sizeof(user_session));
  session->id = id++;
  session->username = NULL;
  session->is_auth = false;
  session->state_machine = new_state_machine();
  session->connection = connection;
  return session;
}

bool start_session(user_session *session) {
  struct parser *command_parser = command_parser_init();

  printf("[CONNECTION #%d] STARTED\n", session->connection);
  while (1) {
    dispatch(session->state_machine, command_parser);

    if (session->state_machine == END)
      break;
  }

  printf("[CONNECTION #%d] ENDED\n", session->connection);
}

user_session *delete_user_session(user_session *session) {
  free(session->username);
  free(session);
}

bool session_authenticate(user_session *session, char *password) {
  if (strcmp(password, "admin") == 0) {
    session->is_auth = true;
    return true;
  }

  return false;
}