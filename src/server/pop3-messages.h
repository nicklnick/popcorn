#ifndef POP3_MESSAGES_H
#define POP3_MESSAGES_H

// AUTHORIZATION
#define USER           "USER"
#define PASS           "PASS"

#define ERR_COMMAND    "-ERR unknown command\r\n"

#define OK_USER        "+OK USER\r\n"
#define OK_PASS        "+OK PASS\r\n"
#define ERR_USER       "-ERR USER\r\n"
#define ERR_PASS_VALID "-ERR PASS: invalid password\r\n"
#define ERR_PASS_LOCK  "-ERR PASS: unable to lock mailbox\r\n"

// TRANSACTION
// FIXME: Remove extra messages where it is not appropriate

#define STAT     "STAT"
#define LIST     "LIST"
#define RETR     "RETR"
#define DELE     "DELE"
#define NOOP     "NOOP"
#define RSET     "RSET"
#define QUIT     "QUIT"

#define OK_STAT  "+OK STAT\r\n"
#define ERR_STAT "-ERR STAT\r\n"

#define OK_LIST  "+OK LIST\r\n"
#define ERR_LIST "-ERR LIST: No such message\r\n"

#define OK_RETR  "+OK RETR\r\n"
#define ERR_RETR "-ERR RETR: No such message\r\n"

#define OK_DELE  "+OK DELE: Message deleted\r\n"
#define ERR_DELE "-ERR DELE: No such message\r\n"

#define OK_NOOP  "+OK NOOP\r\n"

#define OK_RSET  "+OK RSET\r\n"

// UPDATE
#define OK_QUIT  "+OK QUIT\r\n"
#define ERR_QUIT "-ERR QUIT\r\n"

#endif // POP3_MESSAGES_H
