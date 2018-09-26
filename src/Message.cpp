#include "../include/Message.hpp"

using namespace std;
using namespace Dropbox;

Message::Message (string message, int part, int totalParts) {
  this->totalParts = totalParts;
  this->message = message;
  this->part = part;
  this->size = message.length();
  this->type = TYPE_DATA;
}

Message::Message (int type, int part) {
  this->type = type;
  this->part = part;
}

string Message::getMessage() {
  return this->message;
}

int Message::getType() {
  return this->type;
}

int Message::getSize() {
  return this->size;
}

int Message::getPart() {
  return this->part;
}

int Message::getTotalParts() {
  return this->totalParts;
}

void Message::setMessage(string message) {
  this->message = message;
  this->size = message.length();
}

void Message::fillWithZeros(int zeros) {
  for(int i = 0; i < zeros; i++)
    this->message.push_back(0);
}

MessageData * Message::serialize() {
  MessageData * data = new MessageData();
  data->type = this->type;
  data->seq = this->part;
  data->totalSize = this->totalParts;
  data->len = this->size;
  strcpy(data->payload, this->message.c_str());
  // TODO delete this data
  return data;
}
