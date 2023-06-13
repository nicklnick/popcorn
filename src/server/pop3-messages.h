#ifndef POP3_MESSAGES_H
#define POP3_MESSAGES_H

#define USER           "USER"
#define PASS           "PASS"

#define ERR_COMMAND    "-ERR unknown command\r\n"

#define OK_USER        "+OK USER\r\n"
#define OK_PASS        "+OK PASS\r\n"
#define ERR_USER       "-ERR USER\r\n"
#define ERR_PASS_VALID "-ERR PASS: invalid password\r\n"
#define ERR_PASS_LOCK  "-ERR PASS: unable to lock mailbox\r\n"

#endif // POP3_MESSAGES_H
