#ifndef STATE_COMMANDS_H
#define STATE_COMMANDS_H

#include "session/session.h"

/** AUTH */
/** @return len of response */
int auth_user_command(session_ptr session, char *arg, int arg_len, char *response_buff);

/** @return len of response */
int  auth_pass_command(session_ptr session, char *arg, int arg_len, char *response_buff, bool * change_status);

/** TRANSACTION **/
/** @return len of response */
int transaction_stat_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff);

/** @return len of response */
int transaction_dele_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff);

/** @return len of response */
int transaction_rset_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff);

/** @return len of response */
int transaction_list_command(session_ptr session, char * arg, int arg_len, char * response_buff, int buffsize);

/** @return len of response */
int transaction_retr_command(session_ptr session, char * arg, int arg_len, char * response_buff, int buffsize);

/** @return len of response */
int transaction_quit_command(session_ptr session);

#endif // STATE_COMMANDS_H
