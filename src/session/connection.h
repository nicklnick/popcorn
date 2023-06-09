#ifndef CONNECTION_H_
#define CONNECTION_H_

typedef int connection;

connection new_connection();

void send_message(connection connection, char *message);

#endif /* CONNECTION_H_ */