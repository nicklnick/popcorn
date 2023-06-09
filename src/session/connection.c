#include "connection.h"
#include <stdio.h>

static long id = 0;

typedef int _connection;

connection new_connection() { return id++; };

void send_message(connection connection, char *message) {
  printf("#%d: %s\n", connection, message);
}