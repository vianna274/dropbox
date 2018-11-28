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

    
    const string rootDir = "/tmp/DropboxService/";

  private:
    mutex electionMutex;
    mutex propagationMutex;
    mutex portsMutex;
    mutex backupList;
    mutex talkToPrimaryMtx;

    WrapperSocket connectClientSocket;
    WrapperSocket listenToServersSocket;
    WrapperSocket * talkToPrimary;
    int status;
    bool portsAvailable[LAST_PORT - FIRST_PORT + 1];
    vector<User*> users;
    vector<string> backups;
    vector<string> usersIp;
    vector<WrapperSocket*> backupsSockets;
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

    /**
     * Used to make a connection between a backup and the primary server
     */
    void makeConnection();

    /**
     *  Runs on a separate thread to listen to propagations from the primary and election messages from others backups
     */
    void listenToServers(WrapperSocket * socket);
    
    /**
     * The primary server propates a new device connection to backups
     */
    void propagateConnection(string username, string userIp);

    /**
     * The new primary server sends a message to all front ends to let them know the ip of the new primary server
     */
    void propagateNewBoss();

    /**
     * The primary server propagates the file to all backups
     */
    void propagateFile(string filename, string username);

    /**
     * The primary server tells all backups to delete the file
     */
    void propagateDelete(string filename, string username);

    /**
     * Used during election to send a message a ANSWER message on Bully Algorithm
     */
    void answer(string ip);

    /**
     * The election winner informs all remaining backups and client devices that it is the new primary server
     */
    void becomeMain();

    /**
     * Return the IP's of all servers which the ip is greater than this server's IP
     */
    vector<string> getHighers();
 
    /**
     * The backup server starts the election
     */
    void startElection(vector<string> highers);

    /**
     * Sends a ELECTION message to all servers with IP greater than this server's IP
     */
    void sendHighersElection(vector<string> highers);

    /**
     * Wait for other backups answers after an ELECTION message
     */
    bool waitForAnswer();

    /**
     * Wait for other backup to win the election
     */
    bool waitForCoordinator();

    /**
     * Create a new thread and socket to listen to a backup server
     */
    void createNewPortBackup(WrapperSocket * socket);

    /**
     * Remove from the backup list the primary server after he is killed
     */
    void removeFromBackup(vector<string> backups, string main);

    /**
     * The backup connects to the new primary server
     */
    void handleReceiveCoordinator(MessageData * res);
};

}

void synchronize();
#endif
