#include "../include/Client.hpp"
#include <iostream>
#include <fstream>

using namespace std;
using namespace Dropbox;


Client::Client (string username, string serverAddr, int serverDistributorPort) : username(username), syncDirPath("/tmp/sync_dir_"+username)
{

	createSyncDir();

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

void Client::upload(string filePath){

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
	char buffer[MESSAGE_LEN];
	while(packetsSent < totalPackets){
		file.seekg(filePointer, file.beg);
		filePointer += MESSAGE_LEN;
		if(packetsSent == totalPackets - 1) packetSize = lastPacketSize;
		
		file.read(buffer, packetSize);
		MessageData packet = make_packet(TYPE_DATA, packetsSent + 1, totalPackets, packetSize, buffer);
		this->socket->send(&packet);

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
	MessageData message = make_packet(EXIT, 1, 1, -1, "");
	this->socket->send(&message);
}

void Client::triggerNotifications(){
	cout << "Triggering notifications" << "\n";
}