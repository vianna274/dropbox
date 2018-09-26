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
#include "MessageData.hpp"
#include "WrapperSocket.hpp"

using namespace std;

namespace Dropbox {

class Server
{
  public:
    Server();
    int getAvailablePort();
    void sendFile(string filename);

    mutex connectNewClientMutex;
    mutex portsMutex;
    string rootDir = "/home/";

  private:
    WrapperSocket connectClientSocket;
    bool portsAvailable[LAST_PORT - FIRST_PORT + 1];

    void initializePorts();
    void listenToClient(WrapperSocket *socket);
    void connectNewClient();
};

}

#endif
