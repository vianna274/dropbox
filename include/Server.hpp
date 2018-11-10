#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <chrono>
#include <poll.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <fstream>
#include <time.h>
#include <chrono>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "constants.hpp"
#include "MessageData.hpp"
#include "WrapperSocket.hpp"
#include "User.hpp"
#include "FileRecord.hpp"
#include "Operations.hpp"

using namespace std;

/**
 *  Class representing the Server
 * 
 */
namespace Dropbox {

class Server : public Operations
{
  public:
    Server(string ipLocal);
    Server(string ipLocal, string ipMain, vector<string> backups);
    ~Server();

    /**
     *  Run the server, listening to new clients in order to create new connections on a new socket
     */
    void run();

    mutex portsMutex;
    const string rootDir = "/tmp/DropboxService/";

  private:
    WrapperSocket connectClientSocket;
    WrapperSocket listenToBackups;
    WrapperSocket talkToPrimary;
    bool portsAvailable[LAST_PORT - FIRST_PORT + 1];
    vector<User*> users;
    vector<string> backups;
    vector<WrapperSocket> backupsSockets;
    string ipLocal;
    string ipMain;
    bool isMain;


    /**
     *  Return a Port not beign used.
     *  Race condition is protected by portsMutex
     */
    int getAvailablePort();

    /**
     *  Set all port to available
     */
    void initializePorts();

    /**
     *  Set a port to available
     *  Race condition is protected by portsMutex
     */
    void setPortAvailable(int port);

    /**
     *  Check all directories on /tmp/DropboxService/ and create an User for each directory
     */
    void initializeUsers();

    /**
     *  This method is executed in another thread, listening to client requests
     */
    void listenToClient(WrapperSocket *socket, User *user);

    /**
     *  Receive a request to check if a file is outdated
     */
    void receiveAskUpdate(WrapperSocket * socket, User * user);

    /**
     *  Refuse a client if the number of devices being used by the client is bigger than MAX_DEVICES
     */
    void refuseOverLimitClient(User *user);

    /**
     *  Connect a new Client on a new socket with a new port
     */
    void connectNewClient();

    /**
     *  Returns the User from username
     *  Returns nullptr if the User does not exist
     */
    User* getUser(string username);

    /**
     *  Look for the FileRecord given on server files and remove it from files if it doesn't need
     *  to be sent to client. If it needs to be updated the updatedFile parameter is updated.
     */
    int lookForRecordAndRemove(FileRecord file, vector<FileRecord> *files, FileRecord * updatedFile);

    /**
     *  Update oudated or missing client files
     */
    void updateClient(vector<FileRecord> serverFiles, vector<FileRecord> clientFiles, WrapperSocket * socket, User * user);

    /** 
     *  Ends a client session, closing its socket
     */
    void exitUser(WrapperSocket *socket, User *user);
    
    /**
     * Sends the file's filerecord that's within the dirPath through the given socket
     */
    void sendFileRecord(WrapperSocket * socket, string filename, User * user);
};

}

#endif
