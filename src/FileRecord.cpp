#include "../include/FileRecord.hpp"

using namespace std;
using namespace Dropbox;

FileRecord Dropbox::make_record(const char *filename, time_t creationTime, time_t accessTime, time_t modificationTime, int size) {
    
    FileRecord fileRecord;
    strcpy(fileRecord.filename, filename);
    fileRecord.creationTime = creationTime;
    fileRecord.accessTime = accessTime;
    fileRecord.modificationTime = modificationTime;
    fileRecord.size = size;

    return fileRecord;
}