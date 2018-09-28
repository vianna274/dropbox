#ifndef __WRAPPER_SOCKET_HPP__
#define __WRAPPER_SOCKET_HPP__

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

#include "constants.hpp"
#include "MessageData.hpp"

using namespace std;

namespace Dropbox {

class WrapperSocket
{
  public:
    WrapperSocket(string host, int port);
    WrapperSocket(int port);
    ~WrapperSocket();
    void send(MessageData *packet);
    void bindSocket(int port);
    MessageData *receive(int timeout);
    int getPortInt();
    
  private:
    int localSocketHandler;
    int portInt;
    hostent *server;
    sockaddr_in remoteSocketAddr;
    sockaddr_in localSocketAddr;
    socklen_t remoteSocketLen;
    void sendAck(MessageData *ack);
    bool waitAck(int seq);

};

}

#endif
