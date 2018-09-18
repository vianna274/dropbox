#ifndef __MESSAGE_HPP__
#define __MESSAGE_HPP__

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <math.h>

using namespace std;

namespace Dropbox {

class Message
{
  public:
    Message(string message, int part);
    string getMessage();
    void setMessage(string message);
    int getPart();
    int getSize();
    void fillWithZeros(int zeros);
  private:
    string message;
    int size;
    int part;

};

}

#endif