#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

#include <dirent.h>

int get_file_count(DIR *dir);

struct dirent * readdir_files(DIR * dir, int file_pos);

#endif // _FILE_UTILS_H
