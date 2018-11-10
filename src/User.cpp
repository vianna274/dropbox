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
    DIR * dir;
    struct dirent *ent;
    struct stat filestatus;
    if ((dir = opendir (dirPath.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type == 0x8) {
                stat((dirPath + ent->d_name).c_str(), &filestatus);
                FileRecord fileRecord = make_record(ent->d_name, filestatus.st_ctim.tv_sec, filestatus.st_atim.tv_sec, filestatus.st_mtim.tv_sec, filestatus.st_size);
				fileRecords.push_back(fileRecord);
            }
        }
        closedir(dir);
    } 
}

vector<FileRecord> User::getFileRecords() {
    return this->fileRecords;
}

void User::removeFileRecord(string filename) {
	vector<FileRecord>::iterator it;
	for(it = this->fileRecords.begin(); it != this->fileRecords.end(); it++) {
		if (string(it->filename) == filename) {
			this->fileRecords.erase(it);
			return;
		}
	}
}

void User::updateFileRecord(FileRecord newFile) {
	vector<FileRecord>::iterator it;
	for(it = this->fileRecords.begin(); it != this->fileRecords.end(); it++) {
		if (string(it->filename) == string(newFile.filename)) {
			*it = newFile;
			return;
		}
	}
	this->fileRecords.push_back(newFile);
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
