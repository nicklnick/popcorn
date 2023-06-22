#ifndef POPCORN_H
#define POPCORN_H

#include "request_parser.h"

#define NAME_SIZE     16
#define PASSWORD_SIZE 16

void handle_request(popcorn_request *request, popcorn_response *response);

#endif // POPCORN_H
