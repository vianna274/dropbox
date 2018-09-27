#include "../include/Client.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace Dropbox;


Client::Client (string username, string serverAddr, int serverDistributorPort) : username(username), syncDirPath("/tmp/sync_dir_"+username){

	createSyncDir();

	cout << "creating user for " << username << "\n";
	WrapperSocket socketToGetPort(serverAddr, serverDistributorPort);

	MessageData request;
	strcpy(request.payload, username.c_str());
	request.type = TYPE_MAKE_CONNECTION;
	request.seq = 1; request.totalSize = 1;
	socketToGetPort.send(request);
	cout << "REQUEST SENT" << "\n";
	
	MessageData *newPort = socketToGetPort.receive(TIMEOUT_OFF);
	if(newPort->type == TYPE_MAKE_CONNECTION){
		cout << "RECEIVED NEW PORT!";

		this->socket = new WrapperSocket(serverAddr, stoi(newPort->payload));
		cout << "NEW PORT :: " << newPort->payload << "\n";
	}
}

void Client::createSyncDir(){

	struct stat st;
	if(stat(syncDirPath.c_str(), &st) == -1) //Se não existe cria, se existe faz nada
		mkdir(syncDirPath.c_str(), 0777);
}

Client::~Client(){
	delete this->socket;
}


void Client::upload(string filePath){
	cout << "uploading : " << filePath << "\n";
	//TODO: TRATAR PEGAR O SÓ NOME DO FILEPATH

	ifstream file;
	file.open(filePath, ifstream::in | ifstream::binary);
	file.seekg(0, file.end);
	int fileSize = file.tellg();
	file.seekg(0, file.beg);
	int totalPackets = (int)((fileSize/MESSAGE_LEN)+1);
	int lastPacketSize = fileSize % MESSAGE_LEN;
	int packetsSent = 0;
	int filePointer = 0;
	int packetSize = MESSAGE_LEN;
	while(packetsSent < totalPackets){
		file.seekg(filePointer, file.beg);
		filePointer += MESSAGE_LEN;
		if(packetsSent == totalPackets - 1) packetSize = lastPacketSize;
		
		MessageData data;
		file.read(data.payload, packetSize);
		data.len = packetSize;
		data.type = TYPE_DATA;
		data.seq = packetsSent + 1;
		data.totalSize = totalPackets;
		
		this->socket->send(data);

		packetsSent++;
	}
	file.close();
}

void Client::uploadAll(string filePath){
	cout << "uploading ALL : " << filePath << "\n";
}

void Client::download(string filePath){
	cout << "downloading : " << filePath << "\n";
}

void Client::downloadAll(string filePath){
	cout << "downloading ALL : " << filePath << "\n";
}

void Client::updateAll(string filePath){
	cout << "updating ALL : " << filePath << "\n";
}

void Client::del(string filePath){
	cout << "deleting : " << filePath << "\n";
}

void Client::list_server(){
	cout << "listing servers " << "\n";
}

void Client::list_client(){
	cout << "listing clients" << "\n";
}

void Client::get_sync_dir(){
	cout << "getting sync dir" << "\n";
}

void Client::exit(){
	cout << "exiting" << "\n";
}

void Client::triggerNotifications(){
	cout << "Triggering notifications" << "\n";
}