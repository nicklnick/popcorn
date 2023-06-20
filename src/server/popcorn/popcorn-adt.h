#ifndef POPCORN_ADT_H
#define POPCORN_ADT_H

#include "../../selector/selector.h"

typedef struct popcorn *popcorn_ptr;

popcorn_ptr init_popcorn(void);

int get_popcorn_server_sock(void);

int validate_user_pass(const char *user, const char *pass);

struct fd_handler *get_popcorn_sock_fd_handler(void);

void set_popcorn_sock_handlers(void (*handle_read)(struct selector_key *key),
                               void (*handle_write)(struct selector_key *key));

#endif // POPCORN_ADT_H
