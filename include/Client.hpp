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


namespace Dropbox
{
    class Client
    {

        private:
            string username;
            Dropbox::WrapperSocket *socket;
            string syncDirPath;

            void createSyncDir();
        public:
            Client(string username, string serverAddr, int serverDistributorPort);
            ~Client();

            void upload(string filePath);
            void uploadAll(string filePath);
            void download(string filePath);
            void downloadAll(string filePath);
            void updateAll(string filePath);
            void del(string filePath);
            void list_server();
            void list_client();
            void get_sync_dir();
            void exit();
            void triggerNotifications();
    };
}



#endif