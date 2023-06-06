#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pop_parser.h"

// Definición del parser para USER

// Tamaño de un array
#define N(x) (sizeof(x)/sizeof((x)[0]))

enum states{            // global POP3 states
    AUTHORIZATION_INIT,
    AUTHORIZATION_PASS,
    TRANSACTION,
    UPDATE,
    ERROR
};

enum user_states {
    S0,       // U           ->
    S1,       // S           ->
    S2,       // E           ->
    S3,       // R           ->
    S4,       // ' '         ->
    S_ARGS,   // arg_letter+ ->
    S_EQ,     // 
    S_NEQ     // 
};

enum event_type {
    MAY_EQ,
    EQ,
    NEQ
};

static void
copy(struct parser_event *ret, const uint8_t c) {
    ret->type    = MAY_EQ;
    ret->data[ret->n - 1] = c;
    ret->n = ret->n + 1;
}

static void
may_eq(struct parser_event *ret, const uint8_t c) {
    ret->type    = MAY_EQ;
    ret->n       = 1;
    ret->data[0] = c;
}

static void
eq(struct parser_event *ret, const uint8_t c) {
    ret->type    = EQ;
}

static void
neq(struct parser_event *ret, const uint8_t c) {
    ret->type    = NEQ;
    ret->n       = 1;
    ret->data[0] = c;
}

bool is_letter(char c1, char c2){
    return c1 == c2;
}

static void
copy_argument(struct parser_event *ret, const uint8_t c){
    ret->type    = EQ;
    ret->n       = 1;
    ret->data[0] = c;
}

// lowercase y upppercase single letters
// printable ascii characters

static const struct parser_state_transition ST_S0 [] =  {
    {.when = 'U',        .dest = S1,        .act1 = may_eq,},
    {.when = ANY,        .dest = S_NEQ,        .act1 = neq,},
};

static const struct parser_state_transition ST_S1 [] =  {
    {.when = 'S',        .dest = S2,        .act1 = may_eq,},
    {.when = ANY,        .dest = S_NEQ,        .act1 = neq,},
};

static const struct parser_state_transition ST_S2 [] =  {
    {.when = 'E',        .dest = S3,        .act1 = may_eq,},
    {.when = ANY,        .dest = S_NEQ,        .act1 = neq,},
};

static const struct parser_state_transition ST_S3 [] =  {
    {.when = 'R',        .dest = S4,        .act1 = may_eq,},
    {.when = ANY,        .dest = S_NEQ,        .act1 = neq,},
};

static const struct parser_state_transition ST_S4 [] =  {
    {.when = ' ',        .dest = S_ARGS,        .act1 = may_eq,},
    {.when = ANY,        .dest = S_NEQ,        .act1 = copy,},
};

static const struct parser_state_transition ST_ARGS [] =  {
    {.when = '\n',         .dest = S_EQ,        .act1 = eq,},
    {.when = ANY,         .dest = S_ARGS,        .act1 = copy,},
};

 static const struct parser_state_transition ST_EQ [] =  {
    {.when = ANY,        .dest = S_NEQ,       .act1 = neq,},
 };

 static const struct parser_state_transition ST_NEQ [] =  {
    {.when = ANY,        .dest = S_NEQ,       .act1 = neq,},
 };

static const struct parser_state_transition *states [] = {
    ST_S0,
    ST_S1,
    ST_S2,
    ST_S3,
    ST_S4,
    ST_ARGS,
    ST_EQ,
    ST_NEQ
};

static const size_t states_n [] = {
    N(ST_S0),
    N(ST_S1),
    N(ST_S2),
    N(ST_S3),
    N(ST_S4),
    N(ST_ARGS),
    N(ST_EQ),
    N(ST_NEQ),
};

static struct parser_definition user_parser_def = {
    .states_count = N(states),
    .states       = states,
    .states_n     = states_n,
    .start_state  = S0,
};

void clean_buffer(void){
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

enum states parse_user(struct parser *user_parser){
    int c;
    struct parser_event * event = malloc(sizeof(struct parser_event));
    printf("Type USER\n");
    do{
        c = getchar();
        event = parser_feed(user_parser, c);
    }
    while(event->type == MAY_EQ);
    if (event->type == EQ){   
        printf("OK - USER\n");
        (event->data)[event->n - 1] = '\0';
        printf("Your argument is: %s\n", event->data);
        return AUTHORIZATION_PASS;
    }
    else {
        return ERROR;
    }
}

int main(int argc, char const *argv[])
{

    struct parser *user_parser = parser_init(parser_no_classes(), &user_parser_def);
    enum states state = AUTHORIZATION_INIT;
    while(1){
        switch(state){
            case AUTHORIZATION_INIT:{
                state = parse_user(user_parser);
                break;
            }
            case AUTHORIZATION_PASS: {
                printf("OK - NOW PASSWORD\n");
                while(1){

                };
                break;
            }
            case ERROR: {
                printf("ERR - NOT USER\n");
                clean_buffer();
                parser_reset(user_parser);
                state = AUTHORIZATION_INIT;
                break;
            }
            default: {
                printf("UNKNOWN STATE");
            }
        }
    }

    parser_destroy(user_parser);
    return 0;
}
