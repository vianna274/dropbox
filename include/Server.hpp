#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <chrono>
#include <poll.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <mutex>

#include "constants.hpp"
#include "Packet.hpp"
#include "WrapperSocket.hpp"

using namespace std;

namespace Dropbox {

class Server
{
  public:
    Server();
    int getAvailablePort();
    mutex mt;
  private:
    WrapperSocket connectClientSocket;
    bool portsAvailable[6000];
    void listenToClient();
};

}

#endif
