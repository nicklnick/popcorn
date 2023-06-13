#ifndef SM_H_
#define SM_H_

typedef enum state {
    START,
    AUTHORIZATION,
    TRANSACTION,
    END
} state;

#include "../session/session.h"

typedef struct state_machine *state_machine_ptr;

state_machine_ptr new_state_machine();

int dispatch(state_machine_ptr state_machine, session_ptr session,char * buff, int nbytes);

state get_current_state(state_machine_ptr state_machine);

void free_state_machine(state_machine_ptr state_machine);

#endif /* SM_H_ */