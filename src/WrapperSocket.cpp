#include "../include/WrapperSocket.hpp"
#include "../include/MessageData.hpp"

#define ERROR -1

using namespace std;
using namespace Dropbox;

WrapperSocket::WrapperSocket(){}

WrapperSocket::WrapperSocket (string host, int port) {
  this->localSocketHandler = socket(AF_INET, SOCK_DGRAM, 0);
  this->lastData = NULL;
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
  this->lastData = NULL;
  this->localSocketHandler = socket(AF_INET, SOCK_DGRAM, 0);
  if (this->localSocketHandler == ERROR) {
    fprintf(stderr, "Error on creating socket");
    exit(1);
  }
  this->bindSocket(port);
  this->remoteSocketLen = sizeof(struct sockaddr_in);

  this->portInt = port;
}

bool WrapperSocket::send(MessageData *packet, int wait) {
  set_socketSeq(packet, this->socketSeq);
  int tries = 0;
  do{

    if (sendto(this->localSocketHandler, (void *)packet, PACKET_LEN, 0,(const struct sockaddr *) &(this->remoteSocketAddr),sizeof(struct sockaddr_in)) < 0) {
      fprintf(stderr, "Error on sending");
      cout << string(packet->payload) << packet->type << " " << string(packet->username) << endl;
      exit(1);
    }
  
  }while(!this->waitAck(packet->seq, wait) && ++tries < TOTAL_TRIES);
  this->socketSeq++;
  
  if(tries == TOTAL_TRIES) return false;
  return true;
}

bool WrapperSocket::waitAck(int seq, int wait) {
  bool acked = false;
  while(!acked) {
    MessageData *response = this->receive(TIMEOUT_ON, wait);
    if(!response) {
      return false;
    } 
    if(response->type == TYPE_ACK && response->seq == seq) { 
      return true;
    }
  }
  return false;
}

int WrapperSocket::send(MessageData *packet) {
  set_socketSeq(packet, this->socketSeq);
  int tries = 0;
  do{

    if (sendto(this->localSocketHandler, (void *)packet, PACKET_LEN, 0,(const struct sockaddr *) &(this->remoteSocketAddr),sizeof(struct sockaddr_in)) < 0) {
      fprintf(stderr, "Error on sending");
      cout << string(packet->payload) << packet->type << " " << string(packet->username) << endl;
      return -1;
    }
  
  }while(!this->waitAck(packet->seq) && ++tries < TOTAL_TRIES);
  this->socketSeq++;
  
  if(tries == TOTAL_TRIES) return 0;
  return 1;
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

bool WrapperSocket::isEqual(MessageData * a, MessageData * b) {
  if (a->type != b->type || a->seq != b->seq || a->totalSize != b->totalSize)
    return false;
  if (a->len != b->len || a->socketSeq != b->socketSeq)
    return false;
  if (string(a->payload).compare(string(b->payload)) != 0)
    return false;
  if (string(a->username).compare(string(b->username)) != 0)
    return false;
  return true;
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
          cout << "Timeout na porta: " << this->getPortInt() << endl;
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

    if (data->type != TYPE_ACK && this->lastData != NULL && this->isEqual(this->lastData, data))
      continue;
    if (data->socketSeq == -1)
      break;
    if (data->socketSeq == 0)
      this->socketSeq = 0;
    if (data->socketSeq == this->socketSeq && data->type != TYPE_ACK) {
      MessageData message = make_packet(TYPE_ACK, data->seq, 1, -1, "");
      this->sendAck(&message);
      this->socketSeq = data->socketSeq + 1;
      break;
    }
    cout << "Recebi errado na porta: " << this->getPortInt() << " tipo: " << data->type << " data: " << string(data->payload) << endl;
  }

  this->lastData = data;
  // TODO delete msg
  return data;
}

MessageData* WrapperSocket::receive(int timeout, int time) {
  if(timeout == TIMEOUT_ON) {
    struct pollfd fd;
    fd.fd = this->localSocketHandler;
    fd.events = POLLIN;
    int ret = poll(&fd, 1, time);
    switch(ret) {
      case -1: printf("Error\n");
        return NULL;
      case 0: 
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
      this->socketSeq = data->socketSeq + 1;
      receivedCorrectly = true;
    }
  }

  
  // TODO delete msg
  return data;
}

bool WrapperSocket::isElection(int type)
{
  return false;
  // return (type == TYPE_ANSWER || type == TYPE_COORDINATOR || type == TYPE_ELECTION);
}

void WrapperSocket::sendAck(MessageData *ack) {
  if (sendto(this->localSocketHandler, (void *)ack, PACKET_LEN, 0,(const struct sockaddr *) &(this->remoteSocketAddr), sizeof(struct sockaddr_in)) < 0) {
    fprintf(stderr, "Error on sending ACK");
    exit(1);
  }
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
