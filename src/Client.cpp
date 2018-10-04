#include "../include/Client.hpp"
#include "../include/MessageData.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;
using namespace Dropbox;


Client::Client (string username, string serverAddr, int serverDistributorPort) : username(username), syncDirPath("/tmp/sync_dir_"+username+"/")
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
	thread askServerUpdatesThread(&Client::askServerUpdates, this);
	askServerUpdatesThread.detach();
	cout << "Connected Successfully." << endl;
}

void Client::askServerUpdates() {
	while(1) {
		this->mtx.lock();
		this->askUpdate();
		this->mtx.unlock();
		this_thread::sleep_for(chrono::milliseconds(5000));
	}
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

void Client::askUpdate() {
	MessageData request = make_packet(TYPE_REQUEST_UPDATE, 1 , 1, -1, "type_request_update");
	this->socket->send(&request);
	this->sendFileList(this->socket, this->getSyncDirPath(), this->getFileList(this->getSyncDirPath()));
	while(1) {
		MessageData * response = this->socket->receive(TIMEOUT_OFF);
		switch (response->type) {
			case TYPE_NOTHING_TO_SEND:
				break;
			case TYPE_REQUEST_UPDATE_DONE:
				return;
			case TYPE_SEND_UPLOAD_ALL:
				this->receiveUploadAll(this->socket, this->getSyncDirPath());
				break;
			case TYPE_DELETE_ALL:
				this->deleteAll(this->getFileList(this->getSyncDirPath()), this->getSyncDirPath());
				break;
			default:
				cout << "Received " + to_string(response->type) << endl;
		}
	}
}

void Client::download(string filename){
	this->mtx.lock();
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
		this->receiveFile(this->socket, filename, currentPath);
	else if(data->type == TYPE_NOTHING_TO_SEND)
		cout << "File does not exist." << endl;
	this->mtx.unlock();
}

void Client::list_client(){
	cout << "listing clients" << "\n";
}

void Client::get_sync_dir(){
	this->createSyncDir();
	cout << "Getting sync dir" << endl;
}

void Client::exit(){
	MessageData message = make_packet(EXIT, 1, 1, -1, "");
	this->socket->send(&message);
}

void Client::triggerNotifications() {
	cout << "Triggering notifications" << "\n";
}

void Client::requestServerFileList() {
	this->mtx.lock();
	MessageData packet = make_packet(TYPE_LIST_SERVER, 1, 1, -1, "list_server");
	socket->send(&packet);
	vector<FileRecord> files = this->receiveFileList(this->getSocket());
	this->printFileList(files);
	this->mtx.unlock();
}