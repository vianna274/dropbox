#include "../include/MessageData.hpp"

using namespace std;
using namespace Dropbox;

MessageData* Dropbox::make_packet(int type, int seq, int totalSize, int len, const char *payload){
    MessageData *data = (MessageData*) malloc(sizeof(MessageData));
    data->type = type;
    data->seq = seq;
    data->totalSize = totalSize;
    data->len = len;
    strncpy(data->payload, payload, MESSAGE_LEN-1);
    return data;
}