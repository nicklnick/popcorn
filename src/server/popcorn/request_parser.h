#define RES_BUFF_SIZE 256
#define NAME_SIZE     16
#define PASSWORD_SIZE 16
#define REQ_COMMAND_SIZE  20

typedef struct popcorn_request {
    int version;
    char username[NAME_SIZE + 1];
    char password[PASSWORD_SIZE + 1];
    int req_id;
    char command[REQ_COMMAND_SIZE + 1];
    char argument1[REQ_COMMAND_SIZE + 1];
    char argument2[REQ_COMMAND_SIZE + 1];
} popcorn_request;

int parse_request(char *request_str, popcorn_request *request);