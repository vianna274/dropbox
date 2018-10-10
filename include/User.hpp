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
#include <mutex>
#include <dirent.h>
#include "WrapperSocket.hpp"
#include "MessageData.hpp"
#include "FileRecord.hpp"
#include "constants.hpp"

/*
*    Class representing a instance of an user on the server, grouping their devices
*/
namespace Dropbox
{
    class User
    {
        private:
            string username;
            string dirPath;
            vector<FileRecord> fileRecords;
            vector<WrapperSocket*> devices;
            mutex mt;

            /**
            *  Create the user directory on the server if it is a new one, otherwise get the file records of the dir
            */
            void createUserDir();

        public:
            User(string username, string dirPath);
            string getUsername();
            string getDirPath();

            /**
            *  Returns if there is 0, 1 or 2 devices connected
            */
            int getNumDevicesConnected();

            /**
            *  Add the given WrapperSocket to the devices list
            */
            void addDevice(WrapperSocket *socket);

            /**
            *  Remove the given WrapperSocket of the devices list
            */
            void closeDeviceSession(WrapperSocket *socket);

            /**
            *  Lock and unlock functions for the devices mutex
            */
            void lockDevices() { this->mt.lock(); };
            void unlockDevices() { this->mt.unlock(); };
    };
}
#endif