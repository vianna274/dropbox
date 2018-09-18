#include "WrapperSocket.hpp"

#define ERROR -1

using namespace std;
using namespace Dropbox;

WrapperSocket::WrapperSocket (string host, int port) {
  this->sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (this->sockfd == ERROR) {
    fprintf(stderr, "Error on creating socket");
    exit(1);
  }
  this->server = gethostbyname(host.c_str());
  this->serv_addr.sin_family = AF_INET;
  this->serv_addr.sin_port = htons(port);
  this->serv_addr.sin_addr = *((struct in_addr *)this->server->h_addr);
	bzero(&(this->serv_addr.sin_zero), 8);  
}

void WrapperSocket::send(Packet packet) {
  for(int i = 0; i < packet.getSize(); i++) {
    if (sendto(this->sockfd, packet.getIndexMessage(i).c_str(), 
        strlen(packet.getIndexMessage(i).c_str()), 0,
      (const struct sockaddr *) &(this->serv_addr), 
      sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "Error on sending");
        exit(1);
      }
  }
}