#ifndef _LIST_H
#define _LIST_H

#include <string.h>

typedef struct listCDT * listADT;
typedef char * elem_type;

static int compare(elem_type elem1, elem_type elem2) {
    return strcmp(elem1, elem2) < 0;
}

listADT new_list();
void free_list(listADT list);
int is_empty(listADT list);
int size(listADT list);
int belongs(listADT list, elem_type elem);
int insert(listADT list, elem_type elem);
int delete_list(listADT list, elem_type elem);
elem_type get(listADT list, unsigned int i);
void to_begin(listADT list);
int has_next(listADT list);
elem_type next(listADT list);

#endif