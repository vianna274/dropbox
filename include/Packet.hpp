#ifndef __PACKET_HPP__
#define __PACKET_HPP__

#include "Message.hpp"
#include <vector>
#include <string>

#include "constants.hpp"

using namespace std;

namespace Dropbox {

class Packet
{
  public:
    Packet(string message);
    Packet(int type);
    Message * getIndexMessage(int index);
    string getMessage();
    int getSize();

  private:
    vector<Message> splitMessage(string message);

    vector<Message> messages;
};

}

#endif
