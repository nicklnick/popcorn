#ifndef SESSION_H_
#define SESSION_H_

typedef struct client_session *session_ptr;

#include "../parser/command_parser.h"
#include "../selector/selector.h"
#include "../sm/sm.h"
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>

session_ptr new_client_session(int client_socket);

void close_client_session(session_ptr session);

void session_read(struct selector_key *key);

int session_process(session_ptr session);

void session_send_response(struct selector_key *key);

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

fd_handler *get_fd_handler(session_ptr session);

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
