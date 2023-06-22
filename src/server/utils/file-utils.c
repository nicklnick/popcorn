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
    if(file_pos <= 0)
        return NULL;
    struct dirent * dirent;
    int i = 1;
    do {
        dirent = readdir(dir);
        if (dirent != NULL && dirent->d_type == DT_REG && dirent->d_type != DT_DIR){
            i++;
        }
    }
    while(i <= file_pos && dirent != NULL);

    return dirent;
}
