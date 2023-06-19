#include "state-commands.h"
#include "../server/pop3-limits.h"
#include "../server/pop3-messages.h"
#include "../server/server_adt.h"
#include "../utils/file-utils.h"
#include "../utils/staus-codes.h"
#include "wrapper-functions.h"
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int auth_user_command(session_ptr session, char *arg, int arg_len,
                      char *response_buff) {

    pop_action_state(session);

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

    pop_action_state(session);


    //TODO: check this
    char username[NAME_MAX] = {0};
    int username_len = get_username(session, username);
    if (username_len <= 0) {
        int len = strlen(ERR_PASS_VALID);
        strncpy(response_buff, ERR_PASS_VALID, len);
        *change_status = false;
        return len;
    }

    struct user_dir *user_dir = get_user_dir(username, username_len);

    if (user_dir->is_open) {
        int len = strlen(ERR_PASS_LOCK);
        strncpy(response_buff, ERR_PASS_LOCK, len);
        *change_status = false;
        return len;
    }

    if (strcmp(user_dir->password, arg) != 0){
        int len = strlen(ERR_PASS_VALID);
        strncpy(response_buff, ERR_PASS_VALID, len);
        return len;
    }

    struct user_dir * user_d= get_user_dir(username,username_len);
    user_d->is_open = true;

    char mail_dir[MAILPATH_MAX] = {0};
    strcpy(mail_dir, get_mail_dir_path());
    strcat(mail_dir, "/");
    strncat(mail_dir, username, username_len);

    DIR *client_dir = opendir(mail_dir);
    if (client_dir == NULL)
        perror("auth_pass_command()");
    set_client_dir_pt(session, client_dir);

    int len = strlen(OK_PASS);
    strncpy(response_buff, OK_PASS, len);
    *change_status = true;
    return len;
}

static ssize_t get_file_size(const char *mail_dir, const char *filename) {
    struct stat st;
    int mail_len = strlen(mail_dir);
    int file_len = strlen(filename);
    char *file_path =
        (char *)_calloc(mail_len + 1 + file_len + 1, sizeof(char));

    strncpy(file_path, mail_dir, mail_len);
    strcat(file_path, "/"); // 1
    strncat(file_path, filename, file_len + 1);
    if (stat(file_path, &st) < 0) {
        free(file_path);

        perror("stat()");
        exit(EXIT_FAILURE);
    }
    free(file_path);

    return st.st_size;
}


static void get_file_stats(DIR * dir, int * count, int * total_bytes, char * base_path){
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            *count+=1;
            *total_bytes += get_file_size(base_path, entry->d_name);
        }
    }
}

int transaction_stat_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff) {
    pop_action_state(session);

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

    DIR* client_dir = opendir(mail_dir);
    if(!client_dir) {
        printf("error opening client directory");
        exit(EXIT_FAILURE);
    }

    int file_count = 0, size_bytes = 0;
    struct dirent *entry;
    int *client_mails = get_client_dir_mails(session);
    int i = 0;
    while ((entry = readdir(client_dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            if (!client_mails[i]) {
                file_count++;
                size_bytes += get_file_size(mail_dir, entry->d_name);
            }

            i++;
        }
    }
    get_file_stats(client_dir,&file_count,&size_bytes,mail_dir);

    sprintf(response_buff, "%d %d", file_count, size_bytes);

    return strlen(response_buff);
}

int transaction_dele_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff) {

    pop_action_state(session);

    int status = mark_to_delete(session, atoi(arg));
    if (status == ERROR) {
        strcpy(response_buff, ERR_DELE);
        return ERROR;
    }

    return SUCCESS;
}

int transaction_rset_command(session_ptr session, char *arg, int arg_len,
                             char *response_buff) {

    pop_action_state(session);

    unmark_mails(session);
    return SUCCESS;
}

int transaction_list_command(session_ptr session, char * arg, int arg_len, char * response_buff, int buffsize){

    action_state current = pop_action_state(session);

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

    //arg_len has number + '\0'
    if(arg_len > 1){
        msg = strtol(arg,NULL, 10);
        rewinddir(client_dir);
        client_dirent = readdir_files(client_dir,msg);

        if(client_dirent == NULL || msg == 0){
            return sprintf(response_buff,"%s",ERR_LIST);
        }

        strcat(mail_path, client_dirent->d_name);
        stat(mail_path,&f_stat);
        return sprintf(response_buff,OK_LIST_ARG,msg,f_stat.st_size);
    }

    int total_len = 0;
    int current_line_len = 0;
    char aux_buf[RESPONSE_LEN] = {0};
    long last_dir;

    if(current == PROCESS){
        rewinddir(client_dir);
        int total = 0,count = 0;
        get_file_stats(client_dir,&count,&total,mail_path);
        total_len  = sprintf(aux_buf,OK_LIST_NO_ARG,count,total);

        strncpy(response_buff,aux_buf,total_len);
        response_buff[total_len] = '\0';
        rewinddir(client_dir);

        set_client_dir_pt_index(session,1);
    }

    int i = get_client_dir_pt_index(session);
    last_dir = telldir(client_dir);
    client_dirent = readdir(client_dir);

    while (total_len + current_line_len < buffsize && (client_dirent != NULL)){


        if(client_dirent->d_type == DT_DIR){
            client_dirent = readdir(client_dir);
            continue ;
        }

        mail_path[mail_path_base_len] = '\0';
        strcat(mail_path, client_dirent->d_name);
        stat(mail_path,&f_stat);

        current_line_len = sprintf(aux_buf,"%d %d\r\n", i, f_stat.st_size);

        if(current_line_len + total_len < buffsize){
            strncat(response_buff,aux_buf,current_line_len);
            total_len += current_line_len;
            last_dir = telldir(client_dir);
            client_dirent = readdir(client_dir);
            i++;
        }

    }

    if(client_dirent != NULL){
        seekdir(client_dir,last_dir);
        set_client_dir_pt(session, client_dir);
        set_client_dir_pt_index(session,i);
        push_action_state(session,PROCESSING);
        return total_len;
    }

    current_line_len = sprintf(aux_buf,"%s",END_OF_MULTILINE);
    if(total_len + current_line_len < buffsize){
        strncat(response_buff,END_OF_MULTILINE,current_line_len);
        total_len += current_line_len;
    }
    else{
        push_action_state(session,PROCESSING);
    }

    return total_len;
}

int transaction_retr_command(session_ptr session, char * arg, int arg_len, char * response_buff, int buffsize){

    action_state current = pop_action_state(session);

    //arg_len has number + '\0'
    if(arg_len <= 1){
        return sprintf(response_buff,ERR_COMMAND);
    }

    int mail_index =  strtol(arg,NULL, 10);

    DIR * client_dir = get_client_dir_pt(session);
    rewinddir(client_dir);

    struct dirent * client_dirent;

    char mail_path[MAILPATH_MAX] = {0};
    char username[NAME_MAX] = {0};
    int username_len = get_username(session, username);
    strcpy(mail_path, get_mail_dir_path());
    strcat(mail_path, "/");
    strncat(mail_path, username, username_len);
    strcat(mail_path, "/");
    int mail_path_base_len = strlen(mail_path);

    int i = 1;

    while(i <= mail_index && ((client_dirent = readdir(client_dir))!= NULL)){
        if(client_dirent->d_type == DT_REG)
            i++;
    }

    if(client_dirent == NULL){
        return sprintf(response_buff,ERR_RETR);
    }

    struct retr_state * mail_retr = get_session_retr_state(session);
    int buffer_response_index = 0;

    if(current == PROCESS){
        strcat(mail_path, client_dirent->d_name);
        mail_retr->mail_fd = open(mail_path,O_NONBLOCK);
        buffer_response_index= strlen(OK_RETR);
        strncpy(response_buff,OK_RETR,buffer_response_index);
        buffsize -= buffer_response_index;
    }

    char mail_data[BUFFER_SIZE];
    int read_bytes = 0;
    stuffing_state current_state = NONE;

    while (buffer_response_index < buffsize){

        read_bytes = read(mail_retr->mail_fd,mail_data,BUFFER_SIZE-1);
        mail_data[read_bytes] = '\0';
        if(read_bytes == 0)
            break ;

        int data_index = 0;

        for (; data_index < read_bytes && (buffer_response_index < buffsize); ++data_index) {
            switch (current_state) {
                case CR:
                    if(mail_data[data_index] == '\n')
                        current_state = LF;
                    else
                        current_state = NONE;
                    break ;
                case LF:
                    if(mail_data[data_index] == '.'){
                        response_buff[buffer_response_index++] = '.';
                    }
                    current_state = NONE;
                    break ;
                default:
                    if(mail_data[data_index] == '\r')
                        current_state = CR;
            }
            response_buff[buffer_response_index++] = mail_data[data_index];
        }

        if(buffer_response_index == buffsize){
            lseek(mail_retr->mail_fd,data_index-read_bytes,SEEK_CUR);
        }
    }

    if(read_bytes == 0){
        int len = strlen(END_OF_MULTILINE_RETR);
        if(buffer_response_index < buffsize-len){
            strncpy(response_buff+buffer_response_index,END_OF_MULTILINE_RETR,len);
            buffer_response_index+=len;
        }
    }else{
        push_action_state(session,PROCESSING);
    }

    mail_retr->line_state = current_state;
    return buffer_response_index;
}

int transaction_quit_command(session_ptr session){

    DIR * client_dir = get_client_dir_pt(session);
    rewinddir(client_dir);

    char mail_path[MAILPATH_MAX] = {0};
    char username[NAME_MAX] = {0};
    int username_len = get_username(session, username);
    strcpy(mail_path, get_mail_dir_path());
    strcat(mail_path, "/");
    strncat(mail_path, username, username_len);
    strcat(mail_path, "/");
    int mail_path_base_len = strlen(mail_path);

    int * mails = get_client_dir_mails(session);
    int total = get_client_total_mails(session);

    struct dirent * client_dirent;

    int i = 0;
    while(i < total){
        client_dirent = readdir(client_dir);
        if(client_dirent->d_type != DT_REG)
            continue ;
        if(mails[i] == 1){
            mail_path[mail_path_base_len] = '\0';
            strcat(mail_path,client_dirent->d_name);

            int result = remove(mail_path);
            if(result == -1)
                perror("transaction_quit_command()");
        }
        i++;
    }

    struct user_dir * user_d = get_user_dir(username,strlen(username));
    user_d->is_open = false;
    return 0;
}
