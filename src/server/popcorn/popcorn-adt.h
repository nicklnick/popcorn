#ifndef _POPCORN_ADT_H
#define _POPCORN_ADT_H

typedef struct popcorn* popcorn_ptr;


popcorn_ptr init_popcorn(void);

int get_popcorn_server_sock(void);

int validate_user_pass(const char* user, const char* pass);

#endif // _POPCORN_ADT_H
