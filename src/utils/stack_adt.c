#include "stack_adt.h"
#include <stdlib.h>

typedef struct node {
    stack_data_t data;
    struct node * next;
}node;

typedef struct stack_cdt{
    node * top;
} stack_cdt;

stack_adt new_stack_adt(void){
    stack_adt stack = malloc(sizeof (stack_cdt));
    stack->top = NULL;
    return stack;
}

int push(stack_adt stack, stack_data_t data){

    node * top = stack->top;
    node * new = malloc(sizeof(node));
    if(new == NULL)
        return -1;

    new->next = top;
    new->data = data;
    stack->top = new;

    return 0;
}

int pop(stack_adt stack, stack_data_t * data){
    node * top = stack->top;
    if(top == NULL)
        return -1;

    stack->top = top->next;
    *data = top->data;
    return 0;
}