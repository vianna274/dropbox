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
    return username;
}

int User::getNumDevicesConnected(){
    return devices.size();
}

void User::addDevice(WrapperSocket *socket){
    devices.push_back(socket);
}

void User::closeDeviceSession(WrapperSocket *socket){
    for(auto it = devices.begin(); it != devices.end(); it++){
        if(*it == socket){
            devices.erase(it);
            break;
        }
    }
}
