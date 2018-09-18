#ifndef __PACKET_HPP__
#define __PACKET_HPP__

#include "Message.hpp"
#include <vector>
#include <string>

using namespace std;

namespace Dropbox {

class Packet
{
  public:
    Packet(string message);
    string getIndexMessage(int index);
    string getMessage();
    int getSize();

  private:
    vector<Message> splitMessage(string message);

    vector<Message> messages;
};

}

#endif