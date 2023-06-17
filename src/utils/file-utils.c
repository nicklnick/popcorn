#include "file-utils.h"
#include <stdlib.h>
#include <dirent.h>
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
    struct dirent * dirent = readdir(dir);
    int i = 0;
    while(i < file_pos && dirent != NULL){
        if (dirent->d_type == DT_REG){
            i++;
        }
        dirent = readdir(dir);
    }
    return dirent;
}
