#include "popcorn-handler.h"
#include "popcorn-commands.h"
#include "request_parser.h"
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>

void popcorn_read(struct selector_key *key) {
    size_t rsize = 256;
    char rbuffer[rsize];

    struct sockaddr_in client;
    unsigned int client_length = sizeof(client);

    ssize_t nbytes = recvfrom(key->fd, rbuffer, rsize, 0,
                              (struct sockaddr *)&client, &client_length);
    if (nbytes <= 0)
        return;

    popcorn_request *request = malloc(sizeof(popcorn_request));
    if (request == NULL) {
        printf("Error");
        return;
    }

    if (parse_request(rbuffer, request) != 0) {
        printf("Error");
        return;
    }

    printf("REQUEST RECEIVED\nversion: %d\nauth: %s:%s\nreq-id: %d\ncommand: "
           "%s\n",
           request->version, request->username, request->password,
           request->req_id, request->command);

    popcorn_response *response = calloc(1, sizeof(popcorn_response));
    if (response == NULL) {
        printf("Error");
        return;
    }

    handle_request(request, response);

    char wbuffer[256];
    int wbytes =
        snprintf(wbuffer, 256,
                 "popcorn\r\nversion: %d\r\nreq-id: %d\r\nstatus: "
                 "%d\r\n",
                 response->version, response->req_id, response->status);

    if (response->value[0] != '\0')
        wbytes += snprintf(wbuffer + wbytes, 256 - wbytes, "value: %s\r\n",
                 response->value);

    printf("RESPONSE SENT\n%s", wbuffer);

    sendto(key->fd, wbuffer, wbytes, 0, (struct sockaddr *)&client,
           client_length);

    free(response);
    free(request);
}