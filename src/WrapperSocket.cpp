#include "../include/WrapperSocket.hpp"
#include "../include/MessageData.hpp"

#define ERROR -1

using namespace std;
using namespace Dropbox;

WrapperSocket::WrapperSocket (string host, int port) {
  this->localSocketHandler = socket(AF_INET, SOCK_DGRAM, 0);
  if (this->localSocketHandler == ERROR) {
    fprintf(stderr, "Error on creating socket");
    exit(1);
  }
  this->socketSeq = 0;
  this->server = gethostbyname(host.c_str());
  this->remoteSocketAddr.sin_family = AF_INET;
  this->remoteSocketAddr.sin_port = htons(port);
  this->remoteSocketAddr.sin_addr = *((struct in_addr *)this->server->h_addr);
  bzero(&(this->remoteSocketAddr.sin_zero), 8);
  this->remoteSocketLen = sizeof(struct sockaddr_in);

  this->portInt = port;
}

WrapperSocket::~WrapperSocket(){
  close(this->localSocketHandler);
}

WrapperSocket::WrapperSocket (int port) {
  this->socketSeq = 0;
  this->localSocketHandler = socket(AF_INET, SOCK_DGRAM, 0);
  if (this->localSocketHandler == ERROR) {
    fprintf(stderr, "Error on creating socket");
    exit(1);
  }
  this->bindSocket(port);
  this->remoteSocketLen = sizeof(struct sockaddr_in);

  this->portInt = port;
}

void WrapperSocket::send(MessageData *packet) {
  set_socketSeq(packet, this->socketSeq);
  do{

    if (sendto(this->localSocketHandler, (void *)packet, PACKET_LEN, 0,(const struct sockaddr *) &(this->remoteSocketAddr),sizeof(struct sockaddr_in)) < 0) {
      fprintf(stderr, "Error on sending");
      exit(1);
    }

  }while(!this->waitAck(packet->seq));
  this->socketSeq++;
  
}

bool WrapperSocket::waitAck(int seq) {
  bool acked = false;
  while(!acked) {
    MessageData *response = this->receive(TIMEOUT_ON);
    if(!response) {
      return false;
    } 
    if(response->type == TYPE_ACK && response->seq == seq) { 
      return true;
    }
  }
  return false;
}

MessageData* WrapperSocket::receive(int timeout) {
  if(timeout == TIMEOUT_ON) {
    struct pollfd fd;
    fd.fd = this->localSocketHandler;
    fd.events = POLLIN;
    int ret = poll(&fd, 1, 2000);
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
  bool receivedCorrectly = false;
  MessageData * data;
  while(!receivedCorrectly) {

    int msgSize = recvfrom(this->localSocketHandler, (void *) msg, 
      PACKET_LEN, 0, (struct sockaddr *) &this->remoteSocketAddr, &this->remoteSocketLen);
    if (msgSize < 0) {
      fprintf(stderr, "Error on receiving\n");
      exit(1);
    }

    data = (MessageData *) msg;
    if (data->socketSeq == -1)
      break;
    if (data->socketSeq == 0)
      this->socketSeq = 0;
    if (data->socketSeq == this->socketSeq && data->type != TYPE_ACK) {
      MessageData message = make_packet(TYPE_ACK, data->seq, 1, -1, "");
      this->sendAck(&message);
      this->socketSeq++;
      receivedCorrectly = true;
    }
  }

  
  // TODO delete msg
  return data;
}

void WrapperSocket::sendAck(MessageData *ack) {
  if (sendto(this->localSocketHandler, (void *)ack, PACKET_LEN, 0,(const struct sockaddr *) &(this->remoteSocketAddr), sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr, "Error on sending");
    exit(1);
  }
  //printf("ACK sent\n");
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

int WrapperSocket::getPortInt(){
  return this->portInt;
}
