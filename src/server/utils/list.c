#include "list.h"

#include <stdlib.h>
#include <stdio.h>

typedef struct node {
    elem_type head;
    struct node * tail;
} TNode;

typedef TNode * TList;

struct listCDT {
    size_t size;
    TList first;
    TList current;
};

void to_begin(listADT list){
    list->current = list->first;
}

int has_next (const listADT list){
    return list->current != NULL;
}

elem_type next(const listADT list){
    if (!has_next(list)){
        fprintf(stderr, "Error en next\n");
        exit(1);
    }
    elem_type aux= list->current->head;
    list->current = list->current->tail;
    return aux;
}

listADT new_list() {
    return calloc(1, sizeof(struct listCDT));
}

static void free_list_rec(TList first){
    if ( first == NULL)
        return;
    free_list_rec(first->tail);
    free(first);
}
void free_list(listADT list) {
    free_list_rec(list->first);
    free(list);
}
int is_empty(const listADT list) {
    return list->size == 0;
}
int size(const listADT list) {
    return list->size;
}

static int belongs_rec(const TNode * first, elem_type elem) {
    int c;
    if ( first == NULL  || (c = compare(first->head, elem)) > 0)
        return 0;
    if ( c == 0)
        return 1;
    return belongs_rec(first->tail, elem);

}
int belongs(const listADT list, elem_type elem) {
    return belongs_rec(list->first, elem);
}

static TList insert_rec(TList first, elem_type elem, int * flag) {
    int c;
    if (first == NULL || ( c = compare(first->head, elem)) > 0) {
        TList aux = malloc( sizeof(TNode));
        aux->head = elem;
        aux->tail = first;
        *flag = 1;
        return aux;
    }
    if ( c < 0)
        first->tail = insert_rec(first->tail, elem, flag);
    return first;
}

int insert(listADT list, elem_type elem) {
    int flag=0;
    list->first = insert_rec(list->first, elem, &flag);
    list->size += flag;
    return flag;
}

static TList delete_rec(TList first, elem_type elem, int * flag){
    int c;
    if (first == NULL || (c=compare(first->head, elem)) > 0){
        return first;
    }
    if (c == 0){
        TList aux = first->tail;
        free (first);
        *flag = 1;
        return aux;
    }
    first->tail = delete_rec(first->tail, elem, flag);
    return first;
}
int delete_list(listADT list, elem_type elem){
    int flag=0;
    list->first = delete_rec(list->first, elem, &flag);
    list->size -= flag;
    return flag;
}

static elem_type get_rec(const TList list, unsigned int i){
    if (i == 0){
        return list->head;
    }
    return get_rec(list->tail , i-1);
}

elem_type get(const listADT list, unsigned int i){
    if (i >= list->size){
        fprintf(stderr, "Error con el indice");
        exit(1);
    }
    return get_rec(list->first, i);
}

