#ifndef STACK_ADT_H
#define STACK_ADT_H

typedef struct stack_cdt * stack_adt;

//DEFINE THE DATA YOU WANT TO USE
typedef struct {
    int data;
}stack_data_t;

stack_adt new_stack_adt(void);

/**
 * @return
 * -1 if cannot add the element
 * 0 on success
 */
int push(stack_adt, stack_data_t);

/**
 *
 * @param stack
 * @param data
 * The pointer to put the data
 * @return
 * -1 if stack is empty
 * 0 on success
 */
int pop(stack_adt stack, stack_data_t * data);

#endif // STACK_ADT_H
