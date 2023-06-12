/**
 * @file command_parser.h
 * @brief Definition of parser used for POP3 commands.
 * 
 * This parser is only used for separating input into tokens 
 * (1 command and 2 arguments at maximum, as specified in the protocol)
 * 
 * It does not validate neither the command nor the arguments tokenized against POP3.
 * It does validate tokens lengths (4 and 40 maximum characters for command 
 * and arguments, respectively) 
 */
#ifndef _COMMAND_PARSER_H
#define _COMMAND_PARSER_H

#include "parser.h"
#include "../server/utils.h"

enum command_states {
    COMMAND,
    ARGUMENT1,
    ARGUMENT2,
    FINISHED,
};

enum command_event_types {
    MAY_VALID,
    VALID,
    NOT_VALID
};

/**
 * @brief Creates a command parser.
 * 
 * @return The new command parser.
 */
struct parser * command_parser_init ();

struct parser_event * get_command(struct parser_event * event, struct parser *command_parser, struct rw_buffer *buff, size_t count);

#endif /* _COMMAND_PARSER_H */