#ifndef SESSION_H_
#define SESSION_H_

typedef struct client_session *session_ptr;

#include "../parser/command_parser.h"
#include "../sm/sm.h"
#include "connection.h"
#include <stdbool.h>

session_ptr new_client_session(int client_socket);

session_ptr delete_client_session(session_ptr session);

bool start_session(session_ptr session);

struct parser_event *session_read(session_ptr session);

int session_process(session_ptr session);

void session_write(session_ptr session, int wbytes);

state get_session_state(session_ptr session);

struct parser_event *get_session_event(session_ptr session);

bool session_authenticate(session_ptr session, char *password);

#endif /* SESSION_H_ */