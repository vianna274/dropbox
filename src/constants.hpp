#ifndef __CONSTANTS_HPP_
#define __CONSTANTS_HPP_
#define SERVER_ADDR "localhost"
#define SERVER_PORT 4000
#define MESSAGE_LEN 256
#define PACKET_LEN 500
#define TIME_SEND_AGAIN 5000
#define TIMEOUT_ON 1
#define TIMEOUT_OFF 0
enum TYPES
{
  TYPE_DATA,
  TYPE_ACK
};

#endif