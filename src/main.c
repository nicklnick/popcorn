#include "session/session.h"

int main(int argc, char const *argv[]) {

  session_ptr session = new_user_session(1);
  start_session(session);

  return 0;
}
