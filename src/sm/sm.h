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

int dispatch(state_machine_ptr state_machine, struct parser_event *event,char * buff, int nbytes);

state get_current_state(state_machine_ptr state_machine);

void free_state_machine(state_machine_ptr state_machine);

#endif /* SM_H_ */