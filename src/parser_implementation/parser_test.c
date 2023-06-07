#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "command_parser.h"

/** POP3 states for the session */
enum states{
    AUTHORIZATION_INIT,
    AUTHORIZATION_PASS,
    TRANSACTION,
    UPDATE,
    ERROR
};

struct parser_event * get_command(struct parser_event * event, struct parser *command_parser){
    int c;
    do{
        c = getchar();
        event = parser_feed(command_parser, c);
    }
    while(event->type == MAY_VALID);
    return event;
}

bool validate_command(struct parser_event * event){
    return strcmp(event->command, "USER") == 0;
}

int main(int argc, char const *argv[]){

    struct parser *command_parser = command_parser_init();
    enum states state = AUTHORIZATION_INIT;
    struct parser_event * event = malloc(sizeof(struct parser_event));

    while(1){
        switch(state){
            case AUTHORIZATION_INIT:{
                printf("Type USER\n");
                event = get_command(event, command_parser);
                printf("Command: %s\n Argument1: %s\n Argument2: %s\n",
                    event->command, event->argument1, event->argument2);
                parser_reset(command_parser);
                if (validate_command(event)){
                    printf("OK - USER\n");
                    state= AUTHORIZATION_PASS;
                }
                else{
                    printf("ERR - NOT USER\n");
                }
                break;
            }
            case AUTHORIZATION_PASS: {
                printf("OK - NOW PASSWORD\n");
                while(1){};
                break;
            }
            case ERROR: {
                printf("ERR - NOT USER\n");
                parser_reset(command_parser);
                state = AUTHORIZATION_INIT;
                break;
            }
            default: {
                printf("UNKNOWN STATE");
            }
        }
    }

    parser_destroy(command_parser);
    return 0;
}
