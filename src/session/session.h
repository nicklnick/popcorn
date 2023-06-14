#ifndef SESSION_H_
#define SESSION_H_

typedef struct client_session *session_ptr;

#include "../parser/command_parser.h"
#include "../sm/sm.h"
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>

session_ptr new_client_session(int client_socket);

session_ptr delete_client_session(session_ptr session);

bool start_session(session_ptr session);

struct parser_event *session_read(session_ptr session);

int session_process(session_ptr session);

void session_send_response(session_ptr session, int wbytes);

int session_write_response(session_ptr session, char *response,
                           int response_len);

state get_session_state(session_ptr session);

struct parser_event *get_session_event(session_ptr session);

/**
 * @param session
 * @param username dest buffer to copy the username
 * @return -1 if no username. Else returns username length
 */
int get_username(session_ptr session, char *username);

void set_username(session_ptr session, char *username, int len);

void set_client_dir_pt(session_ptr session, DIR *dir);

DIR *get_client_dir_pt(session_ptr session);

bool session_authenticate(session_ptr session, char *password);

void init_client_dir_mails(session_ptr session);

/**
 * @param session
 * @param mail_num Mail to be deleted
 * @return 0 on success, -1 on error
 */
int mark_to_delete(session_ptr session, int mail_num);

/**
 * @brief Unmarks mails to prevent them from being deleted
 */
void unmark_mails(session_ptr session);

#endif /* SESSION_H_ */
