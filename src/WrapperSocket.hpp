#ifndef __WRAPPER_SOCKET_HPP__
#define __WRAPPER_SOCKET_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <chrono>
#include <poll.h>

#include "constants.hpp"
#include "Packet.hpp"

using namespace std;

namespace Dropbox {

class WrapperSocket
{
  public:
    WrapperSocket(string host, int port);
    WrapperSocket(int port);
    void send(Packet packet);
    void bindSocket(int port);
    MessageData * receive(int timeout);
  private:
    int localSocketHandler;
    hostent * server;
    sockaddr_in remoteSocketAddr, localSocketAddr;
    socklen_t remoteSocketLen;
    void sendAck(Message ack);
    bool waitAck(int seq);

};

}

#endif
