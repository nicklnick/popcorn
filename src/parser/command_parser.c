#include "command_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/** Util that calculates length of an array */
#define N(x) (sizeof(x)/sizeof((x)[0]))

/** Actions for each transition */
// TODO: try to make this generic.
// TODO: validate length of commands and arguments
static void
copy_command(struct parser_event *ret, const uint8_t c){
    ret->type    = MAY_VALID;
    ret->command[ret->index++] = c;
    ret->command[ret->index]='\0';
}

static void
copy_argument1(struct parser_event *ret, const uint8_t c){
    ret->type    = MAY_VALID;
    ret->argument1[ret->index++] = c;
    ret->argument1[ret->index]='\0';
}

static void
copy_argument2(struct parser_event *ret, const uint8_t c){
    ret->type    = MAY_VALID;
    ret->argument2[ret->index++] = c;
    ret->argument2[ret->index]='\0';
}

static void
next_token(struct parser_event *ret, const uint8_t c){
    ret->type    = MAY_VALID;
    ret->index = 0;
}

static void
finish(struct parser_event *ret, const uint8_t c){
    ret->type    = VALID;
    ret->index = 0;
}

/** Transitions */
static const struct parser_state_transition ST_COMMAND [] =  {
    {.when = '\n',        .dest = FINISHED,        .action = finish,},
    {.when = ' ',        .dest = ARGUMENT1,        .action = next_token,},
    {.when = ANY,        .dest = COMMAND,        .action = copy_command,},
};

static const struct parser_state_transition ST_ARGUMENT1 [] =  {
    {.when = '\n',        .dest = FINISHED,        .action = finish,},
    {.when = ' ',        .dest = ARGUMENT2,        .action = next_token,},
    {.when = ANY,        .dest = ARGUMENT1,        .action = copy_argument1,},
};

static const struct parser_state_transition ST_ARGUMENT2 [] =  {
    {.when = '\n',        .dest = FINISHED,        .action = finish,},
    {.when = ANY,        .dest = ARGUMENT2,        .action = copy_argument2,},
};

static const struct parser_state_transition *states [] = {
    ST_COMMAND,
    ST_ARGUMENT1,
    ST_ARGUMENT2,
};

static const size_t states_n [] = {
    N(ST_COMMAND),
    N(ST_ARGUMENT1),
    N(ST_ARGUMENT2)
};

static struct parser_definition command_parser_def = {
    .states_count = N(states),
    .states       = states,
    .states_n     = states_n,
    .start_state  = COMMAND,
};

struct parser * command_parser_init (){
    return parser_init(parser_no_classes(), &command_parser_def);
}

struct parser_event * get_command(struct parser_event * event, struct parser *command_parser){
    int c;
    do{
        c = getchar();
        event = parser_feed(command_parser, c);
    }
    while(event->type == MAY_VALID);
    return event;
}
