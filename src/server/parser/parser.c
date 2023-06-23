#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>

#include "parser.h"

struct parser {
    // TODO: probably remove classes in the future
    const unsigned     *classes;
    const struct parser_definition *def;

    unsigned state;
    struct parser_event event;
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
    }
    return ret;
}

void
parser_reset(struct parser *p) {
    p->state   = p->def->start_state;
}

struct parser_event * parser_feed(struct parser *p, const uint8_t c) {
    // TODO: probably remove this
    const unsigned type = p->classes[c];
    
    const struct parser_state_transition *state = p->def->states[p->state];

    const size_t n                              = p->def->states_n[p->state];
    bool matched   = false;

    for(unsigned i = 0; i < n ; i++) {
        const int when = state[i].when;
        if (when  <= 0xFF) {
            matched = (c == when);
        } else if(when == (int) ANY) {
            matched = true;
        } else {
            matched = (type & when);
        }

        if(matched) {
            state[i].action(&p->event, c);
            p->state = state[i].dest;
            break;
        }
    }
    return &p->event;
}

static const unsigned classes[0xFF] = {0x00};

const unsigned *
parser_no_classes(void) {
    return classes;
}

struct parser_event * get_parser_event(struct parser * parser){
    return &parser->event;
}
