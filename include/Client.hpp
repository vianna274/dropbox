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

#include "WrapperSocket.hpp"
#include "MessageData.hpp"
#include "constants.hpp"
#include "FileRecord.hpp"
#include "Operations.hpp"


namespace Dropbox
{
    class Client : public Operations
    {

        private:
            string username;
            Dropbox::WrapperSocket *socket;
            string syncDirPath;

            void createSyncDir();
        public:
            Client(string username, string serverAddr, int serverDistributorPort);
            ~Client();

            void uploadAll(string filePath);
            void download(string filename);
            void downloadAll(string filePath);
            void updateAll(string filePath);
            void list_client();
            void get_sync_dir();
            void exit();
            void triggerNotifications();
            Dropbox::WrapperSocket * getSocket() { return socket; }
            string getSyncDirPath() { return syncDirPath; }
    };
}



#endif