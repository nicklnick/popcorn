#include "file-utils.h"
#include <stdlib.h>
#include <string.h>

int get_file_count(DIR *dir) {
    if (dir == NULL)
        return -1;

    struct dirent *entry;
    int result = 0;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG)
            result++;
    }

    return result;
}

struct dirent * readdir_files(DIR * dir, int file_pos){
    struct dirent * dirent;
    int i = 0;
    while(i <= file_pos){
        dirent = readdir(dir);
        if ((strcmp(".", dirent->d_name) == 0) || (strcmp("..", dirent->d_name) == 0)){
            continue ;
        }
        i++;
    }
    return dirent;
}
