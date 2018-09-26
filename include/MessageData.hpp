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

struct _MessageData
{
    int type;
    int seq;
    int totalSize;
    int len;
    char payload[MESSAGE_LEN];
};

typedef _MessageData MessageData;

}

#endif
