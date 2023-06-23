#define RES_VALUE_SIZE  512
#define RES_COMMAND_SIZE 20

typedef struct popcorn_response {
    int version;
    int req_id;
    int status_code;
    char response_value[RES_VALUE_SIZE];      // value may be optional in response
} popcorn_response;

int parse_response(char *response_str, popcorn_response *response);
