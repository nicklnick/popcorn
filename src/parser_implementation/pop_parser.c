#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "pop_parser.h"

#define ARGUMENT_SIZE 40

/* CDT del parser */
struct parser {
    /** tipificación para cada caracter */
    const unsigned     *classes;
    /** definición de estados */
    const struct parser_definition *def;

    /* estado actual */
    unsigned            state;
    /* buffer para argumento */
    char argument[ARGUMENT_SIZE];
    /* indice del buffer*/
    int index;
    /* evento que se retorna */
    struct parser_event e1;
    /* evento que se retorna */
    struct parser_event e2;
};

void
parser_destroy(struct parser *p) {
    if(p != NULL) {
        free(p);
    }
}

struct parser *
parser_init(const unsigned *classes,
            const struct parser_definition *def) {
    struct parser *ret = malloc(sizeof(*ret));
    if(ret != NULL) {
        memset(ret, 0, sizeof(*ret));
        ret->classes = classes;
        ret->def     = def;
        ret->state   = def->start_state;
        ret->index = 0;
    }
    return ret;
}

void
parser_reset(struct parser *p) {
    p->state   = p->def->start_state;
}

struct parser_event * parser_feed(struct parser *p, const uint8_t c) {
    // tipificación de caracteres (por ahora no tenemos)
    const unsigned type = p->classes[c];

    p->e1.next = p->e2.next = 0;
    
    //recuperamos la transición actual 
    const struct parser_state_transition *state = p->def->states[p->state];

    //cantidad de transiciones del estado
    const size_t n                              = p->def->states_n[p->state];
    bool matched   = false;

    for(unsigned i = 0; i < n ; i++) {
        const int when = state[i].when;
        if (when  <= 0xFF) {
            matched = (c == when);
        } else if(when == ANY) {
            matched = true;
        } else if(when > 0xFF) {
            matched = (type & when);
        } else {
            matched = false;
        }

        if(matched) {
            state[i].act1(&p->e1, c);
            if(state[i].act2 != NULL) {
                p->e1.next = &p->e2;
                state[i].act2(&p->e2, c);
            }
            p->state = state[i].dest;
            break;
        }
    }
    return &p->e1;
}

static const unsigned classes[0xFF] = {0x00};

const unsigned *
parser_no_classes(void) {
    return classes;
}
