#ifndef _FILE_UTILS_H
#define _FILE_UTILS_H

#include <dirent.h>

int get_file_count(DIR *dir);

struct dirent * readdir_files(DIR * dir, int file_pos);

void delete_files_from_dir(DIR *dir, const char *dir_path);

#endif // _FILE_UTILS_H
