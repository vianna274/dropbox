#ifndef __FILE_RECORD_HPP__
#define __FILE_RECORD_HPP__

#include <string>
#include <string.h>

using namespace std;

namespace Dropbox {

  struct _FileRecord
  {
    char filename[20];
    char type[20];
    char date[25];
    int size;
  };

  typedef _FileRecord FileRecord;

  FileRecord make_record(const char *filename, const char *type, char *date, int size);

}

#endif
