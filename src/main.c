#include "session.h"

int main(int argc, char const *argv[]) {

  session_ptr session = new_user_session();
  start_session(session);

  return 0;
}
