#include "WrapperSocket.hpp"

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
    printf("Enviando: %s\n", toSend->payload);
    printf("Enviando: %d\n", toSend->seq);
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
      printf("ACK Received\n");
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
      case -1: printf("\nError\n");
        return NULL;
      case 0: printf("\nTimeout\n");
        return NULL;
      default: break;
    }
  }

  char* msg = new char[PACKET_LEN];
  memset(msg, 0, PACKET_LEN);
	socklen_t sockAddressSize = sizeof(struct sockaddr);

  int msgSize = recvfrom(this->localSocketHandler, (void *) msg, PACKET_LEN, 0,
    (struct sockaddr *) &this->localSocketAddr, &this->remoteSocketLen);
  if (msgSize < 0) {
    fprintf(stderr, "Error on receiving\n");
    exit(1);
  }
  for(int i = 0; i < PACKET_LEN; i++) {
    printf("%c", msg[i]);
  }
  MessageData * data = (MessageData *) msg;
  if(data->type == TYPE_ACK) {
    printf("Received ACK, Part: %d\n", data->seq);
  } else {
    printf("\n\nReceived a datagram\n");
    printf("Type: %d\n", data->type);
    printf("Seq: %d\n", data->seq);
    printf("TotalSize: %d\n", data->totalSize);
    printf("Payload: %s\n", data->payload);
    Message message = Message(TYPE_ACK, data->seq);
    this->sendAck(message);
  }
  // TODO delete msg
  return data;
}

void WrapperSocket::sendAck(Message ack) {
  if (sendto(this->localSocketHandler, (void *)ack.serialize(), PACKET_LEN, 0,
    (const struct sockaddr *) &(this->localSocketAddr),
    sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr, "Error on sending");
    exit(1);
  }
}

void WrapperSocket::bindSocket(int port) {
  this->remoteSocketAddr.sin_family = AF_INET;
	this->remoteSocketAddr.sin_port = htons(port);
	this->remoteSocketAddr.sin_addr.s_addr = INADDR_ANY;
  bzero(&(this->remoteSocketAddr.sin_zero), 8);
  if (bind(this->localSocketHandler, (struct sockaddr *) &this->remoteSocketAddr,
    sizeof(struct sockaddr)) < 0) {
    fprintf(stderr, "Error on binding");
    exit(1);
  }
}
