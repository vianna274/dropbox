#include "../include/Client.hpp"
#include "../include/MessageData.hpp"
#include <iostream>
#include <fstream>
#include <unistd.h>

using namespace std;
using namespace Dropbox;


Client::Client (string username, string serverAddr, int serverDistributorPort, string localIp) : username(username), syncDirPath("/tmp/sync_dir_"+username+"/")
{
	WrapperSocket socketToGetPort(serverAddr, serverDistributorPort);
	this->username = username;
	this->localIp = localIp;
	MessageData request = make_packet(TYPE_MAKE_CONNECTION, 1, 1, this->localIp.length(),this->localIp.c_str(), this->username.c_str());
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
	this->listenToMaster = new WrapperSocket(10000);
	thread listenToMasterThread(&Client::listenMaster, this);
	listenToMasterThread.detach();
	thread askServerUpdatesThread(&Client::askServerUpdates, this);
	askServerUpdatesThread.detach();
	cout << "Connected Successfully." << endl;
}

void Client::listenMaster() {
	while(1) {
		cout << "Esperando alguem trocar o mestre" << endl;
		MessageData * data = this->listenToMaster->receive(TIMEOUT_OFF);
		cout << "Trocando server Master" << endl;
		// this->lockMutex();
		if (data->type == TYPE_NEW_BOSS) {
			cout << "NEW BOSS " << string(data->payload) << endl;
			WrapperSocket socketToGetPort(string(data->payload), 4000);
			MessageData request = make_packet(TYPE_MAKE_CONNECTION, 1, 1, this->localIp.length(), this->localIp.c_str(), this->username.c_str());
			socketToGetPort.send(&request);
			MessageData *newPort = socketToGetPort.receive(TIMEOUT_OFF);
			if(newPort->type == TYPE_MAKE_CONNECTION){
				cout << " EITA " << endl;
				this->socket = new WrapperSocket(string(data->payload), stoi(newPort->payload));
			} 
			else if(newPort->type == TYPE_REJECT_TO_LISTEN) {
				cout << newPort->payload << endl;
				std::exit(1);
				}
		}
		// this->unlockMutex();
		cout << "Server trocado com sucesso, eu acho :D" << endl;
	}
}

void Client::initializeInotify(int *fd, int *wd){
	/* Initialize Inotify*/
	*fd = inotify_init();
	if ( *fd < 0 ) {
		cout << "Couldn't initialize inotify" << endl;
	}
	/* add watch to starting directory */
	*wd = inotify_add_watch(*fd, syncDirPath.c_str(), IN_CLOSE_WRITE | IN_MOVED_FROM | IN_MOVED_TO); 

	if (*wd == -1){
		cout << "Couldn't add watch to " << syncDirPath.c_str() << endl;
	}
}
void Client::askServerUpdates() {
	
	int fd, wd;
	this->initializeInotify(&fd, &wd);
	
	while(1) {
		this->lockMutex();
		
		this->eventsInotify(&fd);
		
		inotify_rm_watch(fd, wd);
		close(fd);

		this->askUpdate();

		int fd, wd;
		this->initializeInotify(&fd, &wd);

		this->unlockMutex();
		this_thread::sleep_for(chrono::milliseconds(5000));
	}
	/* Clean up*/
	inotify_rm_watch(fd, wd);
	close(fd);
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
	MessageData request = make_packet(TYPE_REQUEST_UPDATE, 1 , 1, -1, "type_request_update", this->username.c_str());
	bool ass = this->socket->send(&request);
	if (ass == false)
		return;
	this->sendFileList(this->socket, this->fileRecords);
	while(1) {
		MessageData * response = this->socket->receive(TIMEOUT_ON);
		FileRecord fileRecord;
		if (response == NULL) return;
		switch (response->type) {
			case TYPE_NOTHING_TO_SEND:
				break;
			case TYPE_REQUEST_UPDATE_DONE:
				return;
			case TYPE_DELETE:
				this->deleteFile(this->getSyncDirPath() + string(response->payload));
				this->removeFileRecord(string(response->payload));
				break;
			case TYPE_SEND_FILE:
				fileRecord = *((FileRecord *)response->payload);
				this->receiveFile(this->socket, string(fileRecord.filename), 
					this->getSyncDirPath());
				this->updateFileRecord(fileRecord);
				break;
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

void Client::removeFileRecord(string filename) {
	vector<FileRecord>::iterator it;
	for(it = this->fileRecords.begin(); it != this->fileRecords.end(); it++) {
		if (string(it->filename) == filename) {
			this->fileRecords.erase(it);
			return;
		}
	}
}

void Client::updateFileRecord(FileRecord newFile) {
	vector<FileRecord>::iterator it;
	for(it = this->fileRecords.begin(); it != this->fileRecords.end(); it++) {
		if (string(it->filename) == string(newFile.filename)) {
			*it = newFile;
			return;
		}
	}
	this->fileRecords.push_back(newFile);
}

void Client::eventsInotify(int* fd){
	int length, i = 0;
	char buffer[BUF_LEN];
	struct pollfd pfd = { *fd, POLLIN, 0 };
	int ret = poll(&pfd, 1, 50);  // timeout of 50ms
	if (ret < 0) {
		fprintf(stderr, "poll failed: %s\n", strerror(errno));
	} else if (ret == 0) {
		// Timeout with no events, move on.
	} else {
		// Process the new event.
		length = read(*fd, buffer, BUF_LEN);  
		while (i < length) {
			struct inotify_event *event = ( struct inotify_event * ) &buffer[ i ];
			if (event->len ) {
				string filename(event->name);
				string path = syncDirPath + filename;
				vector<FileRecord> tempFiles = this->getFileList(this->getSyncDirPath());
				if(event->mask & IN_MOVED_TO){
					FileRecord newRecord = this->sendFileClient(this->socket, path.c_str(), this->username);
					this->updateFileRecord(newRecord);
				}
			
				if(event->mask & IN_MOVED_FROM || event->mask & IN_DELETE){
					this->sendDeleteFile(this->socket, filename.c_str(), this->username);
					this->removeFileRecord(filename);
				}  
		
				if(event->mask & IN_CLOSE_WRITE){
					this->sendDeleteFile(this->socket, filename.c_str(), this->username);
					FileRecord newRecord = this->sendFileClient(this->socket, path.c_str(), this->username);
					this->updateFileRecord(newRecord);
				} 
	
			i += EVENT_SIZE + event->len;
			}
		}
	}
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
	if(data->type == TYPE_SEND_FILE) {
		FileRecord * fileRecord = (FileRecord *)(data->payload);
		this->receiveFile(this->socket, string(fileRecord->filename), currentPath);
	}
	else if(data->type == TYPE_NOTHING_TO_SEND)
		cout << "File does not exist." << endl;
}

void Client::list_client(){
	printFileList(this->fileRecords);
}

void Client::get_sync_dir(){

	this->createSyncDir();
	
	DIR * dir;
    struct dirent *ent;
    if ((dir = opendir (this->getSyncDirPath().c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type == 0x8) {
                deleteFile(this->getSyncDirPath()+string(ent->d_name));
            }
        }
        closedir(dir);
    } 
	cout << "Deleted all files." << endl;
	MessageData packet = make_packet(TYPE_LIST_SERVER, 1, 1, -1, "list_server");
	socket->send(&packet);
	this->fileRecords = this->receiveFileList(this->getSocket());
	for(FileRecord record : this->fileRecords){
		MessageData request = make_packet(TYPE_REQUEST_DOWNLOAD, 1 , 1, -1, record.filename);
		this->socket->send(&request);
		MessageData *data = this->socket->receive(TIMEOUT_OFF);
		if(data->type == TYPE_SEND_FILE) {
			this->receiveFile(this->socket, record.filename, this->getSyncDirPath());
		}
	}
	this->printFileList(this->fileRecords);
	cout << "Received all files." << endl;
}

void Client::exit(){
	MessageData message = make_packet(EXIT, 1, 1, -1, "");
	this->socket->send(&message);
}

void Client::requestServerFileList() {

	MessageData packet = make_packet(TYPE_LIST_SERVER, 1, 1, -1, "list_server");
	socket->send(&packet);
	vector<FileRecord> files = this->receiveFileList(this->getSocket());
	this->printFileList(files);
}
