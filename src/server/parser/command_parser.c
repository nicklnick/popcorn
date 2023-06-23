#include "command_parser.h"
#include "parser.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** Util that calculates length of an array */
#define N(x) (sizeof(x) / sizeof((x)[0]))

/** Actions for each transition */
static void copy_command(struct parser_event *ret, const uint8_t c) {
    ret->type = MAY_VALID;
    ret->command[ret->index++] = c;
    ret->cmd_len++;
    ret->command[ret->index] = '\0';
}

static void copy_argument1(struct parser_event *ret, const uint8_t c) {
    ret->type = MAY_VALID;
    ret->argument1[ret->index++] = c;
    ret->arg1_len++;
    ret->argument1[ret->index] = '\0';
}

static void copy_argument2(struct parser_event *ret, const uint8_t c) {
    ret->type = MAY_VALID;
    ret->argument2[ret->index++] = c;
    ret->arg2_len++;
    ret->argument2[ret->index] = '\0';
}

static void next_token(struct parser_event *ret, const uint8_t c) {
    ret->type = MAY_VALID;
    ret->index = 0;
}

static void finishing(struct parser_event *ret, const uint8_t c) {
    ret->type = MAY_VALID;
    ret->index = 0;
    ret->cmd_len++;
    ret->arg1_len++;
    ret->arg2_len++;
}

static void finish(struct parser_event *ret, const uint8_t c) {
    ret->type = VALID;
}

/** Transitions */
static const struct parser_state_transition ST_COMMAND[] = {
    {
        .when = '\r',
        .dest = FINISHING,
        .action = finishing,
    },
    {
        .when = ' ',
        .dest = ARGUMENT1,
        .action = next_token,
    },
    {
        .when = ANY,
        .dest = COMMAND,
        .action = copy_command,
    },
};
static const struct parser_state_transition ST_ARGUMENT1[] = {
    {
        .when = '\r',
        .dest = FINISHING,
        .action = finishing,
    },
    {
        .when = ' ',
        .dest = ARGUMENT2,
        .action = next_token,
    },
    {
        .when = ANY,
        .dest = ARGUMENT1,
        .action = copy_argument1,
    },
};

static const struct parser_state_transition ST_ARGUMENT2[] = {
    {
        .when = '\r',
        .dest = FINISHING,
        .action = finishing,
    },
    {
        .when = ANY,
        .dest = ARGUMENT2,
        .action = copy_argument2,
    },
};

static const struct parser_state_transition ST_FINISHING[] = {
    {
        .when = '\n',
        .dest = FINISHED,
        .action = finish,
    }
};

static const struct parser_state_transition *states[] = {
    ST_COMMAND,
    ST_ARGUMENT1,
    ST_ARGUMENT2,
    ST_FINISHING
};

static const size_t states_n[] = {N(ST_COMMAND), N(ST_ARGUMENT1),
                                  N(ST_ARGUMENT2), N(ST_FINISHING)};

static struct parser_definition command_parser_def = {
    .states_count = N(states),
    .states = states,
    .states_n = states_n,
    .start_state = COMMAND,
};

struct parser *command_parser_init() {
    return parser_init(parser_no_classes(), &command_parser_def);
}

void command_parser_reset(struct parser *parser) {
    parser_reset(parser);
    struct parser_event *event = get_parser_event(parser);
    event->type = MAY_VALID;
    event->index = 0;
    event->arg1_len = 0;
    event->arg2_len = 0;
    event->cmd_len = 0;
    memset(event->command,'\0',COMMAND_SIZE);
    memset(event->argument1,'\0',ARGUMENT_SIZE);
    memset(event->argument2,'\0',ARGUMENT_SIZE);
}

struct parser_event *get_command(struct parser_event *event,
                                 struct parser *command_parser, char *buff,
                                 size_t count, size_t *nread) {

    size_t i;
    for (i = 0; i < count && event->type == MAY_VALID; i++) {
        event = parser_feed(command_parser, buff[i]);
    }
    *nread = i;
    return event;
}

struct parser_event *get_command_parser_event(struct parser *parser) {
    return get_parser_event(parser);
}

void command_parser_destroy(struct parser *parser) {
    if (parser != NULL)
        parser_destroy(parser);
}
