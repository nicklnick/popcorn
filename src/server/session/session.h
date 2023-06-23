#ifndef SESSION_H_
#define SESSION_H_

typedef struct client_session *session_ptr;

#include "../parser/command_parser.h"
#include "../selector/selector.h"
#include "../sm/sm.h"
#include <dirent.h>
#include <stdbool.h>
#include <sys/types.h>

typedef enum {READ = 0, READING, PROCESS, PROCESSING, WRITE, WRITING} action_state;

typedef enum stuffing_state {CR,LF,DOT,NONE} stuffing_state;

struct retr_state{
    int mail_fd;
    stuffing_state line_state;
};


session_ptr new_client_session(int client_socket);

void close_client_fd_handler (struct selector_key *key);

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

int get_client_dir_pt_index(session_ptr session);

void set_client_dir_pt_index(session_ptr session, int index);

/**
 * @return mail[] with client mails,
 * if mail[i] == 1 then message i was deleted
 */
int *get_client_dir_mails(session_ptr session);

/**
 * @return amount of mails for current client
 */
int get_client_total_mails(session_ptr session);

fd_handler *get_fd_handler(session_ptr session);

struct retr_state * get_session_retr_state(session_ptr session);

void init_client_dir_mails(session_ptr session);

action_state pop_action_state(session_ptr session);

action_state peek_action_state(session_ptr session);

void push_action_state(session_ptr session, action_state action);

/**
 * @param session
 * @param mail_num Mail to be deleted
 * @return 0 on success, -1 on error
 *
 * @note It is the caller´s duty to check if mail_num is valid
 */
int mark_to_delete(session_ptr session, int mail_num);

/**
 * @brief Unmarks mails to prevent them from being deleted
 */
void unmark_mails(session_ptr session);

#endif /* SESSION_H_ */
