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
  if (host != "none") {
    this->server = gethostbyname(host.c_str());
    this->serverAddr.sin_family = AF_INET;
    this->serverAddr.sin_port = htons(port);
    this->serverAddr.sin_addr = *((struct in_addr *)this->server->h_addr);
  	bzero(&(this->serverAddr.sin_zero), 8);
  } else
    this->bindSocket(port);
}

void WrapperSocket::send(Packet packet) {
  for(int i = 0; i < packet.getSize(); i++) {
    MessageData * toSend = packet.getIndexMessage(i)->serialize();
    printf("Enviando: %s\n", toSend->payload);
    printf("Enviando: %d\n", toSend->seq);
    if (sendto(this->sockfd, (void *)toSend, PACKET_LEN, 0,
        (const struct sockaddr *) &(this->serverAddr),
        sizeof(struct sockaddr_in)) < 0) {
        fprintf(stderr, "Error on sending");
        exit(1);
      }
  }
}

void WrapperSocket::receive() {
  char* msg = new char[PACKET_LEN];
  memset(msg, 0, PACKET_LEN);
	socklen_t sockAddressSize = sizeof(struct sockaddr);

  int msgSize = recvfrom(this->sockfd, (void *) msg, PACKET_LEN, 0,
    (struct sockaddr *) &this->clientAddr, &this->client);
  if (msgSize < 0) {
    fprintf(stderr, "Error on receiving");
    exit(1);
  }
  for(int i = 0; i < PACKET_LEN; i++) {
    printf("%c", msg[i]);
  }
  MessageData * data = (MessageData *) msg;
  printf("\n\nReceived a datagram\n");
  printf("Type: %d\n", data->type);
  printf("Seq: %d\n", data->seq);
  printf("TotalSize: %d\n", data->totalSize);
  printf("Payload: %s\n", data->payload);
  // TODO delete msg
}

void WrapperSocket::bindSocket(int port) {
  this->serverAddr.sin_family = AF_INET;
	this->serverAddr.sin_port = htons(port);
	this->serverAddr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(this->serverAddr.sin_zero), 8);
  if (bind(this->sockfd, (struct sockaddr *) &this->serverAddr,
    sizeof(struct sockaddr)) < 0) {
    fprintf(stderr, "Error on binding");
    exit(1);
  }
  this->client = sizeof(struct sockaddr_in);
}
