#include "file-utils.h"
#include <stdlib.h>

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
