#ifndef __FILE_RECORD_HPP__
#define __FILE_RECORD_HPP__

#include <string>
#include <string.h>

using namespace std;

namespace Dropbox {

  struct _FileRecord
  {
    char filename[200];
    char type[10];
    char date[25];
    int size;
  };

  typedef _FileRecord FileRecord;

  FileRecord make_record(const char *filename, const char *type, char *date, int size);

}

#endif
