#include "../include/MessageData.hpp"

using namespace std;
using namespace Dropbox;

MessageData Dropbox::make_packet(int type, int seq, int totalSize, int len, const char *payload){
    MessageData data;
    data.type = type;
    data.seq = seq;
    data.totalSize = totalSize;
    data.len = len;
    // File or record packets can't have \0 at the end of the payload
    if(type == TYPE_DATA || type == TYPE_SEND_FILE)
        memcpy(data.payload, payload, len);
    else
        strncpy(data.payload, payload, MESSAGE_LEN);

    return data;
}