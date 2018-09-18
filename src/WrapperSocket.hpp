#ifndef __WRAPPER_SOCKET_HPP__
#define __WRAPPER_SOCKET_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "Packet.hpp"

using namespace std;

namespace Dropbox {

class WrapperSocket
{
  public:
    WrapperSocket(string host, int port);
    void send(Packet packet);
  private:
    int sockfd;
    hostent * server;
    sockaddr_in serv_addr;

};

}

#endif