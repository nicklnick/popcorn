#ifndef SESSION_H_
#define SESSION_H_

#include "connection.h"
#include <stdbool.h>

typedef struct user_session *session_ptr;

session_ptr new_user_session(connection connection);

session_ptr delete_user_session(session_ptr session);

bool start_session(session_ptr session);

bool session_authenticate(session_ptr session, char *password);

#endif /* SESSION_H_ */