#ifndef __OPERATIONS_HPP__
#define __OPERATIONS_HPP__

#include <fstream>
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
    void sendFileList(WrapperSocket * socket, string dirPath, vector<FileRecord> files);
    void sendUpload(WrapperSocket * socket, string filePath);
    void sendDeleteFile(WrapperSocket * socket, string filePath);
    void sendDeleteDir(WrapperSocket * socket, string dirPath);
    void sendUpdateFile(WrapperSocket * socket, string filePath);

    void receiveUpload(WrapperSocket * socket, string filename, string dirPath);
    void receiveFileList(WrapperSocket * socket);
    void receiveDeleteFile(WrapperSocket * socket, string filename, string dirPath);
    void receiveDeleteDir(WrapperSocket * socket, string dirPath);
    void receiveUpdateFile(WrapperSocket * socket, string filename, string dirPath);
};
}

#endif