#ifndef STATE_COMMANDS_H
#define STATE_COMMANDS_H

#include "../session/session.h"

// We ignore any extra argument
int auth_user_command(session_ptr session, char *arg, int arg_len, char *response_buff);

void auth_pass_command(session_ptr session, char *arg, int arg_len, char *response_buff);

#endif // STATE_COMMANDS_H
