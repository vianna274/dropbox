#include "../include/Client.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;
using namespace Dropbox;


Client::Client (string username, string serverAddr, int serverDistributorPort) : username(username), syncDirPath("/tmp/sync_dir_"+username)
{
	WrapperSocket socketToGetPort(serverAddr, serverDistributorPort);

	MessageData request = make_packet(TYPE_MAKE_CONNECTION, 1, 1, -1, username.c_str());
	socketToGetPort.send(&request);
	
	MessageData *newPort = socketToGetPort.receive(TIMEOUT_OFF);
	if(newPort->type == TYPE_MAKE_CONNECTION){
		this->socket = new WrapperSocket(serverAddr, stoi(newPort->payload));
	} 
	else if(newPort->type == TYPE_REJECT_TO_LISTEN) {
		cout << newPort->payload << endl;
		std::exit(1);
	}

	get_sync_dir();

	cout << "Connected Successfully." << endl;
}

void Client::createSyncDir(){
	struct stat st;
	if(stat(syncDirPath.c_str(), &st) == -1){ //Se não existe cria, se existe faz nada
		mkdir(syncDirPath.c_str(), 0777);
		cout << "Created Sync Dir on " + syncDirPath << endl;
	}
}

Client::~Client(){
	delete this->socket;
}

void Client::uploadAll(string filePath){
	cout << "uploading ALL : " << filePath << "\n";
}

void Client::download(string filename){

	MessageData request = make_packet(TYPE_REQUEST_DOWNLOAD, 1 , 1, -1, filename.c_str());
	this->socket->send(&request);

	char cCurrentPath[FILENAME_MAX];
	if (!getcwd(cCurrentPath, sizeof(cCurrentPath))) 
		cout << "ERROR GETTING THE CURRENT DIRECTORY!!" << endl;
	cCurrentPath[sizeof(cCurrentPath) - 1] = '\0';
	string currentPath(cCurrentPath);
	currentPath += "/";

	MessageData *data = this->socket->receive(TIMEOUT_OFF);
	if(data->type == TYPE_SEND_FILE)
		this->receiveUpload(this->socket, filename, currentPath);
	else if(data->type == TYPE_NOTHING_TO_SEND)
		cout << "File does not exist." << endl;
}

void Client::downloadAll(string filePath){
	cout << "downloading ALL : " << filePath << "\n";
}

void Client::updateAll(string filePath){
	cout << "updating ALL : " << filePath << "\n";
}

void Client::list_client(){
	cout << "listing clients" << "\n";
}

void Client::get_sync_dir(){
	this->createSyncDir();
	this->receiveUploadAll(this->socket, this->syncDirPath);
	cout << "Getting sync dir" << endl;
}

void Client::exit(){
	MessageData message = make_packet(EXIT, 1, 1, -1, "");
	this->socket->send(&message);
}

void Client::triggerNotifications(){
	cout << "Triggering notifications" << "\n";
}