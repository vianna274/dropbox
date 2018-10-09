#ifndef __MESSAGE_DATA_HPP__
#define __MESSAGE_DATA_HPP__

#include <string>
#include <vector>
#include <iostream>
#include <math.h>
#include <string.h>

#include "constants.hpp"

using namespace std;

namespace Dropbox {

/*
*   Struct representing a Packet send on Sockets
*/
struct _MessageData
{
    int type;           // See constants.hpp to check Type
    int seq;            // Sequence number 1...*
    int totalSize;      // Total number of packets that are going to be sent, used to check if it is the last packet
    int len;            // Lenght of the payload
    char payload[MESSAGE_LEN];  // Bytes sent
};

typedef _MessageData MessageData;

/**
 *  Packet constructor.
 *  IMPORTANT:
 *  if type == TYPE_DATA or type == TYPE_SEND_FILE
 *  it will be used memcpy to copy the bytes,
 *  otherwise it will be used strcpy.
 */
MessageData make_packet(int type, int seq, int totalSize, int len, const char *payload);

}

#endif
