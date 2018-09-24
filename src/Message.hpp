#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <string>
#include <vector>
#include <iostream>
#include <math.h>
#include <string.h>

#include "constants.hpp"

using namespace std;

namespace Dropbox {

struct _MessageData
{
    int type;
    int seq;
    int totalSize;
    int len;
    char payload[MESSAGE_LEN];
};

typedef _MessageData MessageData;

class Message
{
  public:
    Message(string message, int part, int totalParts);
    string getMessage();
    void setMessage(string message);
    int getPart();
    int getTotalParts();
    int getSize();
    void fillWithZeros(int zeros);
    MessageData * serialize();
  private:
    string message;
    int size;
    int part;
    int totalParts;
};

}

#endif
