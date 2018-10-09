#ifndef __FILE_RECORD_HPP__
#define __FILE_RECORD_HPP__

#include <string>
#include <string.h>
#include <time.h>

using namespace std;

/*
*    Struct representing a File Record on Client or Server
*/
namespace Dropbox {

  struct _FileRecord
  {
    char filename[200];
    time_t creationTime;      // MAC times from unix system
    time_t accessTime;
    time_t modificationTime;
    int size;
  };

  typedef _FileRecord FileRecord;

  /*
      Create a FileRecord
  */
  FileRecord make_record(const char *filename, time_t creationTime, time_t accessTime, time_t modificationTime, int size);

}

#endif
