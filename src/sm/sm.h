#ifndef SM_H_
#define SM_H_

#include "../parser/command_parser.h"

typedef enum state {
    START,
    AUTHORIZATION,
    TRANSACTION,
    END
} state;

typedef struct state_machine *state_machine_ptr;

state_machine_ptr new_state_machine();

void dispatch(state_machine_ptr state_machine, struct parser *parser);

void free_state_machine(state_machine_ptr state_machine);

#endif /* SM_H_ */