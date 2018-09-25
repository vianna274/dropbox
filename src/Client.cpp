#include "Client.hpp"

using namespace std;
using namespace Dropbox;


Client::Client (string username, WrapperSocket socket) : username(username), socket(socket){
	cout << "creating user for " << username << "\n";
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