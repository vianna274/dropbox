#ifndef __OPERATIONS_HPP__
#define __OPERATIONS_HPP__

#include <fstream>
#include <stdio.h>
#include <dirent.h>
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
    void sendUpload(WrapperSocket * socket, string filePath);
    void sendDeleteFile(WrapperSocket * socket, string filename);
    void sendDeleteDir(WrapperSocket * socket, string dirPath);
    void sendUpdateFile(WrapperSocket * socket, string filePath);
    void sendUploadAll(WrapperSocket * socket, string dirPath, vector<FileRecord> files);
    void sendDeleteAll(WrapperSocket * socket);

    void receiveUpload(WrapperSocket * socket, string filename, string dirPath);
    vector<FileRecord> receiveFileList(WrapperSocket * socket);
    void receiveDeleteFile(WrapperSocket *socket, string filename, string dirPath);
    void receiveDeleteDir(WrapperSocket * socket, string dirPath);
    void receiveUpdateFile(WrapperSocket * socket, string filename, string dirPath);
    void receiveUploadAll(WrapperSocket * socket, string dirPath);
};
}

#endif