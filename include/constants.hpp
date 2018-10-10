#ifndef __CONSTANTS_HPP_
#define __CONSTANTS_HPP_
#define SERVER_ADDR "localhost"
#define SERVER_PORT 4000
#define MESSAGE_LEN 4000
#define PACKET_LEN 4096
#define TIME_SEND_AGAIN 5000
#define TIMEOUT_ON 1
#define TIMEOUT_OFF 0

#define FIRST_PORT 4001
#define LAST_PORT 6000

#define MAX_DEVICES 2

/* for inotify */
#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 200 /*Assuming that the length of the filename won't exceed 16 bytes*/
#define EVENT_SIZE  ( sizeof (struct inotify_event) ) /*size of one event*/
#define BUF_LEN     ( MAX_EVENTS * ( EVENT_SIZE + LEN_NAME )) /*buffer to store the data of events*/
/* for inofity */


enum TYPES
{
  TYPE_DATA,
  TYPE_ACK,
  TYPE_MAKE_CONNECTION,
  TYPE_SEND_FILE,
  TYPE_SEND_FILE_NO_RECORD,
  TYPE_REJECT_TO_LISTEN,
  TYPE_LIST_SERVER,
  TYPE_NOTHING_TO_SEND,
  TYPE_REQUEST_DOWNLOAD,
  TYPE_SEND_UPLOAD_ALL,
  TYPE_SEND_UPLOAD_ALL_DONE,
  TYPE_REQUEST_UPLOAD_ALL,
  TYPE_DELETE,
  TYPE_DELETE_ALL,
  TYPE_REQUEST_UPDATE,
  TYPE_REQUEST_UPDATE_DONE,
  OK,
  DELETE,
  UPDATE,
  CREATE,
  EXIT
};

#endif
