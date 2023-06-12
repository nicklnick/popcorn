/**
 * @file parser.h
 * 
 * @brief Generic parser inspired by @jcodagnone's implementation
 * 
 */
#ifndef POP_PARSER_H_
#define POP_PARSER_H_

#include <stdint.h>
#include <stddef.h>

#define COMMAND_SIZE 5
#define ARGUMENT_SIZE 40

struct parser_event {
    unsigned type;

    // TODO: change this
    char command[COMMAND_SIZE];
    char argument1[ARGUMENT_SIZE];
    char argument2[ARGUMENT_SIZE];

    unsigned short int index;

    int bytes_recv;
};

struct parser_state_transition {
    int       when;
    unsigned  dest;

    void    (*action)(struct parser_event *ret, const uint8_t c);
    
};


struct parser_definition {
    const unsigned                         states_count;
    /** transitions for each state */
    const struct parser_state_transition **states;
    /** number of states per transition */
    const size_t                          *states_n;
    const unsigned                         start_state;
};

struct parser *
parser_init    (const unsigned * classes, const struct parser_definition *def);

void
parser_destroy  (struct parser *p);

/**
 * @brief Resets parser to its initial state.
 * 
 * @param p The parser to be reset.
 * @return Void 
 */
void parser_reset    (struct parser *p);

struct parser_event * parser_feed (struct parser *p, const uint8_t c);

static const unsigned ANY = 1 << 9;

// TODO: probably remove this
const unsigned * parser_no_classes(void);


#endif