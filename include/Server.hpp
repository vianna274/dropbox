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
#include "User.hpp"

using namespace std;

namespace Dropbox {

class Server
{
  public:
    Server();
    int getAvailablePort();

    mutex connectNewClientMutex;
    mutex portsMutex;
    string rootDir = "/home/";

  private:
    WrapperSocket connectClientSocket;
    bool portsAvailable[LAST_PORT - FIRST_PORT + 1];
    vector<User*> users;

    void initializePorts();
    void listenToClient(WrapperSocket *socket);
    void connectNewClient();
    User* getUser(string username);
};

}

#endif
