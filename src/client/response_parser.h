#define RES_VALUE_SIZE  256

typedef struct popcorn_response {
    int version;
    int req_id;
    int status_code;
    char response_value[RES_VALUE_SIZE + 1]; // optional
} popcorn_response;

int parse_response(char *response_str, popcorn_response *response);