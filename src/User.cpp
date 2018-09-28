#include "../include/User.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace Dropbox;

User::User(string username, string dirPath): username(username), dirPath(dirPath){
    createUserDir();
}

void User::createUserDir(){
    struct stat st;
	if(stat(dirPath.c_str(), &st) == -1) //Se não existe cria, se existe faz nada
		mkdir(dirPath.c_str(), 0777);
}

string User::getUsername(){
    return this->username;
}

string User::getDirPath(){
    return this->dirPath;
}

int User::getNumDevicesConnected(){
    return this->devices.size();
}

void User::addDevice(WrapperSocket *socket){
    this->devices.push_back(socket);
}

void User::closeDeviceSession(WrapperSocket *socket){
    for(auto it = this->devices.begin(); it != this->devices.end(); it++){
        if(*it == socket){
            this->devices.erase(it);
            break;
        }
    }
}
