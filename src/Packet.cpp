#include "Packet.hpp"

using namespace std;
using namespace Dropbox;

Packet::Packet(string message) {
  this->messages = this->splitMessage(message);
}

int Packet::getSize() {
  return this->messages.size();
}

vector<Message> Packet::splitMessage(string message) {
  size_t len = message.length();
  double splitLength = 256; // should be a constant
  size_t numOfMessages = ceil(len/splitLength);
  vector<Message> messages;
  int gap = 0;

  for(size_t i = 0; i < numOfMessages; i++) {
    gap = (i*splitLength + splitLength) - len;
    if ((i * splitLength) >= len) {
      splitLength = len;
    }
    Message newMessage(message.substr(i * splitLength, splitLength), (i+1));

    if (gap > 0)
      newMessage.fillWithZeros(gap);
    messages.push_back(newMessage);
  }
  return messages;
}

string Packet::getIndexMessage(int index) {
  if (index >= this->getSize()) {
    fprintf(stderr, "Error trying to getIndexMessage");
    return NULL;
  }
  return this->messages[index].getMessage();
}