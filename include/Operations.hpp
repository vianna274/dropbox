#ifndef __OPERATIONS_HPP__
#define __OPERATIONS_HPP__

#include <fstream>
#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>
#include <iomanip>

#include "WrapperSocket.hpp"
#include "FileRecord.hpp"

using namespace std;

namespace Dropbox {
class Operations
{
  public:
    Operations();
    void deleteFile(string filepath);
    void deleteAll(vector<FileRecord> files, string dirPath);
    vector<FileRecord> getFileList(string dirPath);

    void sendNothing(WrapperSocket * socket);

    void sendFileList(WrapperSocket * socket, string dirPath, vector<FileRecord> files);
    void sendFile(WrapperSocket * socket, string filePath);
    void sendDeleteFile(WrapperSocket * socket, string filename);
    void sendUploadAll(WrapperSocket * socket, string dirPath, vector<FileRecord> files);
    void sendDeleteAll(WrapperSocket * socket);

    void receiveFile(WrapperSocket * socket, string filename, string dirPath);
    vector<FileRecord> receiveFileList(WrapperSocket * socket);
    void printFileList(vector<FileRecord> fileRecords);
    void receiveUploadAll(WrapperSocket * socket, string dirPath);
};
}

#endif