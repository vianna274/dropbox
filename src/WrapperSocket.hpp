#ifndef __WRAPPER_SOCKET_HPP__
#define __WRAPPER_SOCKET_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include "constants.hpp"
#include "Packet.hpp"

using namespace std;

namespace Dropbox {

class WrapperSocket
{
  public:
    WrapperSocket(string host, int port);
    void send(Packet packet);
    void bindSocket(int port);
    void receive();
  private:
    int sockfd;
    hostent * server;
    sockaddr_in serverAddr, clientAddr;
    socklen_t client;

};

}

#endif
