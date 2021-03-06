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

/*
*    Class for the socket functions
*/
namespace Dropbox {

class WrapperSocket
{
  public:
    /*
    *    Constructor for the client, connecting to the given host
    */
    WrapperSocket(string host, int port);
    
    /*
    *    Constructor for the server
    */
    WrapperSocket(int port);
    WrapperSocket();
    ~WrapperSocket();

    /*
    *    Send a packet
    */
    int send(MessageData *packet);
    bool send(MessageData *packet, int wait);

    /*
    *    Bind socket
    */
    void bindSocket(int port);

    /*
    *    Receive a datagram and sends and ack if it is necessary
    */
    MessageData *receive(int timeout);
    MessageData *receive(int timeout, int time);
    int getPortInt();

  private:
    int localSocketHandler;
    int portInt;
    int socketSeq;
    MessageData * lastData;
    hostent *server;
    sockaddr_in remoteSocketAddr;
    sockaddr_in localSocketAddr;
    socklen_t remoteSocketLen;
    void sendAck(MessageData *ack);
    bool waitAck(int seq);
    bool waitAck(int seq, int wait);
    int getPort() { return portInt; }
    bool isElection(int type);
    bool isEqual(MessageData * a, MessageData * b);

};

}

#endif
