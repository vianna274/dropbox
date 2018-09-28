#include "../include/FileRecord.hpp"

using namespace std;
using namespace Dropbox;

FileRecord Dropbox::make_record(const char *filename, const char *type, char *date, int size) {
    
    FileRecord fileRecord;
    strcpy(fileRecord.date, date);
    strcpy(fileRecord.filename, filename);
    strcpy(fileRecord.type, type);
    fileRecord.size = size;

    return fileRecord;
}