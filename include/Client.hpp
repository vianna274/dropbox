#ifndef __CLIENT_HPP_
#define __CLIENT_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>
#include <dirent.h>
#include <iomanip>
#include <thread>
#include <mutex>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/inotify.h>
#include <limits.h>

#include "WrapperSocket.hpp"
#include "MessageData.hpp"
#include "constants.hpp"
#include "FileRecord.hpp"
#include "Operations.hpp"

/*
*    Class representing the Client on the client-side of the System.
*    The sync directory will be created on path /tmp/sync_dir_<username>
*/
namespace Dropbox
{
    class Client : public Operations
    {

        private:
            string username;
            string localIp;
            Dropbox::WrapperSocket *socket;
            Dropbox::WrapperSocket *listenToMaster;
            string syncDirPath;
            mutex mtx;
            mutex socketMtx;
            vector<FileRecord> fileRecords;

            /**
             *  This method is executed in another thread, treating the inotify and the updates from the server
             */
            void askServerUpdates();
            /**
             *  Create the sync directory on path /tmp/sync_dir_<username>
             */
            void createSyncDir();

        public:
            Client(string username, string serverAddr, int serverDistributorPort, string localIp);
            ~Client();

            string getUsername() { return username; }
            /**
             *  Send a TYPE_REQUEST_UPDATE packet for the server and deals with the response to syncronize the local dir with the server
             */
			string payload1;
			int payload2;
            void askUpdate();

            /**
             *  Process the detected inotify events
             */
            void eventsInotify(int* fd);

            /**
             *  Initialize the inotify and add the watch to the local dir
             */
            void initializeInotify(int *fd, int *wd);

            /**
             *  Download filename from server
             */
            void download(string filename);

            /**
             *  Print the file records of the client
             */
            void list_client();

            /**
             * Delete all files from local dir and download all from server
             */
            void get_sync_dir();

            /**
             * End session
             */
            void exit();

            /**
             * Prints the server file record list
             */
            void requestServerFileList();

            /**
             * Remove the file record of filename from the list
             */
            void removeFileRecord(string filename);

            /**
             * Update filerecord on the list
             */
            void updateFileRecord(FileRecord newFile);
            
            /**
             * Return the Client's socket
             */
            Dropbox::WrapperSocket * getSocket() { return socket; }

            /**
             * Return full path of syncDir folder
             */
            string getSyncDirPath() { return syncDirPath; }
            
            /**
             * Mutex to synchronize update sending and keyboard commnands 
             */
            void lockMutex() { this->mtx.lock(); };
            void unlockMutex() { this->mtx.unlock(); };

            /**
             * Keep listening on port 10000 for a primary server change
             */          
            void listenMaster();
    };
}



#endif
