#include "../include/Client.hpp"

using namespace std;
using namespace Dropbox;


Client::Client (string username, string serverAddr, int serverDistributorPort) : username(username){
	cout << "creating user for " << username << "\n";
	WrapperSocket socketToGetPort(serverAddr, serverDistributorPort);
	Packet packet = Packet(TYPE_MAKE_CONNECTION);
	socketToGetPort.send(packet);

	MessageData * newPort = socketToGetPort.receive(TIMEOUT_OFF);
	cout << "RECEIVED NEW PORT!" << endl;

	this->socket = new WrapperSocket(serverAddr, stoi(newPort->payload));
	cout << "NEW PORT :: " << newPort->payload << "\n";

}

Client::~Client(){
	delete this->socket;
}

void Client::upload(string filePath){
	cout << "uploading : " << filePath << "\n";
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