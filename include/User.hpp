#ifndef __USER_HPP_
#define __USER_HPP_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>
#include <dirent.h>
#include "WrapperSocket.hpp"
#include "MessageData.hpp"
#include "constants.hpp"


namespace Dropbox
{
    class User
    {
        private:
            string username;
            string dirPath;
            vector<int> files; //CRIAR TIPO FILE
            vector<WrapperSocket*> devices;

            void createUserDir();

        public:
            User(string username, string dirPath);
            string getUsername();
            int getNumDevicesConnected();
            void addDevice(WrapperSocket *socket);

    };
}



#endif