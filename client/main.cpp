#include <iostream>
//
#include <libviewer/socket.hpp>

using namespace std;
using namespace viewer;

int main(int argc, char* argv[]) {
  if (1 == argc) return 0;

  string message = argv[1];
  for (size_t i = 2; i < argc; ++i) {
    message += ' ';
    message += argv[i];
  }

  client_socket client{"/tmp/libviewer-server.socket"};
  client.write(message);
}
