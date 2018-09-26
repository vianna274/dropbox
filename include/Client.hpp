#ifndef __CLIENT_HPP_
#define __CLIENT_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include "WrapperSocket.hpp"
#include "Packet.hpp"
#include "constants.hpp"


namespace Dropbox
{
    class Client
    {

        private:
            string username;
            Dropbox::WrapperSocket *socket;
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