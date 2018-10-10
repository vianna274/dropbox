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

/**
 *  Generic class that implements operations with files.
 *  Both Client and Server extends this class. 
 */
namespace Dropbox {
class Operations
{
  public:
    Operations();

    /**
     *  Return the FileRecords for all the files in the given directory
     */
    vector<FileRecord> getFileList(string dirPath);

    /**
     *  Return the FileRecord of the specific filename
     */
    FileRecord getRecord(vector<FileRecord> files, string filename);

    /**
     *  Remove the file
     */
    void deleteFile(string filepath);

    /**
     *  Remove all files from the given directory
     */
    void deleteAll(vector<FileRecord> files, string dirPath);

    /**
     *  Send a packet of TYPE_NOTHING_TO_SEND
     */
    void sendNothing(WrapperSocket * socket);

    /**
     *  Send TYPE_DATA packets containing each FileRecord of the list
     */
    void sendFileList(WrapperSocket * socket, vector<FileRecord> files);

    /**
     *  Send a TYPE_SEND_FILE with the file record and then send the whole file in TYPE_DATA packets
     */
    void sendFile(WrapperSocket * socket, string filePath, FileRecord fileRec);

    /**
     *  Send a TYPE_SEND_FILE_NO_RECORD and then send the whole file in TYPE_DATA packets
     */
    void sendFile(WrapperSocket * socket, string filePath);

    /**
     *  Send a TYPE_DELETE packet with the name of the file to delete
     */
    void sendDeleteFile(WrapperSocket * socket, string filename);

    /**
     *  Send all the files on the dirPath with their file records
     */
    void sendUploadAll(WrapperSocket * socket, string dirPath, vector<FileRecord> files);

    /**
     *  Send a TYPE_DELETE_ALL packet
     */
    void sendDeleteAll(WrapperSocket * socket);

    /**
     *  Receive a file and create it in filename
     */
    void receiveFile(WrapperSocket * socket, string filename, string dirPath);

    /**
     *  Receive and return a file records list
     */
    vector<FileRecord> receiveFileList(WrapperSocket * socket);

    /**
     *  Print the given file records list
     */
    void printFileList(vector<FileRecord> fileRecords);

    /**
     *  Receive all files
     */
    void receiveUploadAll(WrapperSocket * socket, string dirPath);
};
}

#endif