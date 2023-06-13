#include "state-commands.h"
#include "../server/pop3-messages.h"
#include "../server/server_adt.h"
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

int auth_user_command(session_ptr session, char *arg, int arg_len, char *response_buff) {

    struct user_dir * user_dir = get_user_dir(arg,arg_len);

    //User does not exist
    if(user_dir == NULL){
        int len = strlen(ERR_USER);
        strncpy(response_buff,ERR_USER,len);
        return len;
    }

    set_username(session,arg,arg_len);
    int len = strlen(OK_USER);
    strncpy(response_buff,OK_USER,len);

    return len;
}

void auth_pass_command(session_ptr session, char *arg, int arg_len, char *response_buff) {
    char username[256] = {0};
    int username_len = get_username(session, username);
    if (username_len <= 0) {
        int len = strlen(ERR_PASS_VALID);
        strncpy(response_buff,OK_USER,len);
    }

    //BUSCAR CONTRASEÑA DE USER
    // COMPARAR CONTRA ARG
    // NO: ERROR
    // SI: LOCK DEL STATUS DEL DIR

}
