#include "../include/WrapperSocket.hpp"

#define ERROR -1

using namespace std;
using namespace Dropbox;

WrapperSocket::WrapperSocket (string host, int port) {
  this->localSocketHandler = socket(AF_INET, SOCK_DGRAM, 0);
  if (this->localSocketHandler == ERROR) {
    fprintf(stderr, "Error on creating socket");
    exit(1);
  }
  this->server = gethostbyname(host.c_str());
  this->remoteSocketAddr.sin_family = AF_INET;
  this->remoteSocketAddr.sin_port = htons(port);
  this->remoteSocketAddr.sin_addr = *((struct in_addr *)this->server->h_addr);
  bzero(&(this->remoteSocketAddr.sin_zero), 8);
  this->remoteSocketLen = sizeof(struct sockaddr_in);

}

WrapperSocket::~WrapperSocket(){
  close(this->localSocketHandler);
}

WrapperSocket::WrapperSocket (int port) {
  this->localSocketHandler = socket(AF_INET, SOCK_DGRAM, 0);
  if (this->localSocketHandler == ERROR) {
    fprintf(stderr, "Error on creating socket");
    exit(1);
  }
  this->bindSocket(port);
  this->remoteSocketLen = sizeof(struct sockaddr_in);

}

void WrapperSocket::send(Packet packet) {
  bool sendAgain = false;
  for(int i = 0; i < packet.getSize(); i++) {
    MessageData * toSend = packet.getIndexMessage(i)->serialize();
    printf("Enviando Type: %d\n", toSend->type);
        printf("Enviando Type: %s\n", toSend->payload);

    if (sendto(this->localSocketHandler, (void *)toSend, PACKET_LEN, 0,
      (const struct sockaddr *) &(this->remoteSocketAddr),
      sizeof(struct sockaddr_in)) < 0) {
      fprintf(stderr, "Error on sending");
      exit(1);
    }

    sendAgain = !this->waitAck(toSend->seq);
    if (sendAgain) {
      i--;
      sendAgain = false;
    }
  }
}

bool WrapperSocket::waitAck(int seq) {
  bool acked = false;
  while(!acked) {
    MessageData * response = this->receive(TIMEOUT_ON);
    if(!response) {
      return false;
    } 
    if(response->type == TYPE_ACK && response->seq == seq) { 
      return true;
    }
  }
}

MessageData * WrapperSocket::receive(int timeout) {
  if(timeout == TIMEOUT_ON) {
    struct pollfd fd;
    fd.fd = this->localSocketHandler;
    fd.events = POLLIN;
    int ret = poll(&fd, 1, 500);
    switch(ret) {
      case -1: printf("Error\n");
        return NULL;
      case 0: printf("Timeout\n");
        return NULL;
      default: break;
    }
  }

  char* msg = new char[PACKET_LEN];
  memset(msg, 0, PACKET_LEN);
	socklen_t sockAddressSize = sizeof(struct sockaddr);

  int msgSize = recvfrom(this->localSocketHandler, (void *) msg, 
    PACKET_LEN, 0, (struct sockaddr *) &this->remoteSocketAddr, &this->remoteSocketLen);
  if (msgSize < 0) {
    fprintf(stderr, "Error on receiving\n");
    exit(1);
  }

  MessageData *data = (MessageData *) msg;
  printf("Received a datagram Type: %d\n", data->type);
  if(data->type == TYPE_ACK) {
    printf("Received ACK\n");
  } else {
    Message message = Message(TYPE_ACK, data->seq);
    printf("Sending a ACK\n");
    this->sendAck(message);
  }
  // TODO delete msg
  return data;
}

void WrapperSocket::sendAck(Message ack) {
  if (sendto(this->localSocketHandler, (void *)ack.serialize(), PACKET_LEN, 0,
    (const struct sockaddr *) &(this->remoteSocketAddr),
    sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr, "Error on sending");
    exit(1);
  }
  printf("ACK sent\n");
}


void WrapperSocket::bindSocket(int port) {
  this->remoteSocketAddr.sin_family = AF_INET;
	this->remoteSocketAddr.sin_port = htons(port);
	this->remoteSocketAddr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(this->remoteSocketAddr.sin_zero), 8);
  if (bind(this->localSocketHandler, (struct sockaddr *) &this->remoteSocketAddr, sizeof(struct sockaddr)) < 0) {
    fprintf(stderr, "Error on binding");
    exit(1);
  }
}
