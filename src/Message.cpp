#include "Message.hpp"


using namespace std;
using namespace Dropbox;

Message::Message (string message, int part) {
  this->message = message;
  this->part = part; // TODO
  this->size = message.length();
}

string Message::getMessage() {
  return this->message;
}

int Message::getSize() {
  return this->size;
}

int Message::getPart() {
  return this->part;
}

void Message::setMessage(string message) {
  this->message = message;
  this->size = message.length();
}

void Message::fillWithZeros(int zeros) {
  for(int i = 0; i < zeros; i++)
    this->message.push_back(0);
}