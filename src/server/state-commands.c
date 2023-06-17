#include "state-commands.h"
#include "../server/pop3-limits.h"
#include "../server/pop3-messages.h"
#include "../server/server_adt.h"
#include "../utils/staus-codes.h"
#include "../utils/file-utils.h"
#include "wrapper-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

int auth_user_command(session_ptr session, char *arg, int arg_len,
                      char *response_buff) {

    struct user_dir *user_dir = get_user_dir(arg, arg_len);

    // User does not exist
    if (user_dir == NULL) {
        int len = strlen(ERR_USER);
        strncpy(response_buff, ERR_USER, len);
        return len;
    }

    set_username(session, arg, arg_len);
    int len = strlen(OK_USER);
    strncpy(response_buff, OK_USER, len);

    return len;
}

int auth_pass_command(session_ptr session, char *arg, int arg_len, char *response_buff, bool * change_status) {
    char username[NAME_MAX] = {0};
    int username_len = get_username(session, username);
    if (username_len <= 0) {
        int len = strlen(ERR_PASS_VALID);
        strncpy(response_buff,ERR_PASS_VALID,len);
        *change_status = false;
        return len;
    }

    struct user_dir *user_dir = get_user_dir(username, username_len);

    if (user_dir->is_open) {
        int len = strlen(ERR_PASS_LOCK);
        strncpy(response_buff,ERR_PASS_LOCK,len);
        *change_status = false;
        return len;
    }

    // BUSCAR CONTRASEÑA DE USER
    //  COMPARAR CONTRA ARG
    //  NO: ERROR

    char mail_dir[MAILPATH_MAX] = {0};
    strcpy(mail_dir, get_mail_dir_path());
    strcat(mail_dir, "/");
    strncat(mail_dir, username, username_len);

    DIR *client_dir = opendir(mail_dir);
    if(client_dir == NULL)
        perror("auth_pass_command()");
    set_client_dir_pt(session, client_dir);

    int len = strlen(OK_PASS);
    strncpy(response_buff,OK_PASS,len);
    *change_status = true;
    return len;

}

static ssize_t get_file_size(const char *mail_dir, const char *filename) {
    struct stat st;
    char *file_path =
        _malloc((strlen(mail_dir) + strlen(filename) + 1) * sizeof(char));

    strcpy(file_path, mail_dir);
    strcat(file_path, "/");
    strncat(file_path, filename, strlen(filename));
    if (stat(file_path, &st) < 0) {
        free(file_path);

        perror("stat()");
        exit(EXIT_FAILURE);
    }
    free(file_path);

    return st.st_size;
}

int transaction_stat_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff) {
    DIR *client_dir = get_client_dir_pt(session);

    if (!client_dir) {
        perror("Error opening directory");
        exit(EXIT_FAILURE);
    }

    // get mail_dir
    char username[NAME_MAX] = {0};
    int username_len = get_username(session, username);
    if (username_len < 0) {
        printf("Error getting username");
        exit(EXIT_FAILURE);
    }
    char mail_dir[MAILPATH_MAX] = {0};
    strcpy(mail_dir, get_mail_dir_path());
    strcat(mail_dir, "/");
    strncat(mail_dir, username, username_len);

    int file_count = 0, size_bytes = 0;
    struct dirent *entry;
    while ((entry = readdir(client_dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            file_count++;
            size_bytes += get_file_size(mail_dir, entry->d_name);
        }
    }

    sprintf(response_buff, "%d %d", file_count, size_bytes);

    return strlen(response_buff);
}

int transaction_dele_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff) {

    int status = mark_to_delete(session, atoi(arg));
    if (status == ERROR) {
        strcpy(response_buff, ERR_DELE);
        return ERROR;
    }

    return SUCCESS;
}

int transaction_rset_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff) {
    unmark_mails(session);
    return SUCCESS;
}

int transaction_list_command(session_ptr session, char * arg, int arg_len, char * response_buff, int buffsize){

    DIR * client_dir = get_client_dir_pt(session);
    long msg;
    struct dirent * client_dirent;

    char mail_path[MAILPATH_MAX] = {0};
    char username[NAME_MAX] = {0};
    int username_len = get_username(session, username);
    strcpy(mail_path, get_mail_dir_path());
    strcat(mail_path, "/");
    strncat(mail_path, username, username_len);
    strcat(mail_path, "/");
    int mail_path_base_len = strlen(mail_path);

    struct stat f_stat;

    if(arg_len > 1){
        msg = strtol(arg,NULL, 10);
        rewinddir(client_dir);
        client_dirent = readdir_files(client_dir,msg);
        strcat(mail_path, client_dirent->d_name);

        stat(mail_path,&f_stat);

        sprintf(response_buff,OK_LIST_ARG,msg,f_stat.st_size);
        return strlen(response_buff);
    }

    int total_len = 0;
    int current_line_len = 0;
    char aux_buf[RESPONSE_LEN] = {0};

    total_len = current_line_len = sprintf(aux_buf,OK_LIST_NO_ARG,0,0);
    strncpy(response_buff,aux_buf,current_line_len);
    response_buff[current_line_len] = '\0';

    int i = 0;
    long last_dir = 0;

    rewinddir(client_dir);
    last_dir = telldir(client_dir);
    client_dirent = readdir(client_dir);

    while (total_len + current_line_len < buffsize && client_dirent != NULL){

        if(client_dirent->d_type == DT_DIR){
            client_dirent = readdir(client_dir);
            continue ;
        }

        mail_path[mail_path_base_len] = '\0';
        strcat(mail_path, client_dirent->d_name);
        stat(mail_path,&f_stat);

        current_line_len = sprintf(aux_buf,"%d %d\r\n", i++, f_stat.st_size);

        if(current_line_len + total_len < buffsize){
            strncat(response_buff,aux_buf,current_line_len);
            total_len += current_line_len;
        }

        last_dir = telldir(client_dir);
        client_dirent = readdir(client_dir);
    }

    seekdir(client_dir,last_dir);

    return total_len;
}
