#include "../include/User.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace Dropbox;

User::User(string username): username(username){

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