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

namespace Dropbox {

class Server : public Operations
{
  public:
    Server();
    int getAvailablePort();

    mutex portsMutex;
    const string rootDir = "/tmp/DropboxService/";

  private:
    WrapperSocket connectClientSocket;
    bool portsAvailable[LAST_PORT - FIRST_PORT + 1];
    vector<User*> users;

    void initializePorts();
    void setPortAvailable(int port);
    void initializeUsers();
    void listenToClient(WrapperSocket *socket, User *user);
    void receiveAskUpdate(WrapperSocket * socket, User * user);
    void refuseOverLimitClient(User *user);
    void connectNewClient();
    User* getUser(string username);
    int findRecord(FileRecord file, vector<FileRecord> *files);
    void updateClient(vector<FileRecord> serverFiles, vector<FileRecord> clientFiles, 
      WrapperSocket * socket, User * user);
    void exitUser(WrapperSocket *socket, User *user);
};

}

#endif
