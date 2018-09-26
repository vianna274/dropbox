#include "../include/Server.hpp"

using namespace std;
using namespace Dropbox;

Server::Server() : connectClientSocket(SERVER_PORT){
    for(int i = 4001; i < 6000; i++){
        portsAvailable[i] = true;
    }

    while(true){
        this->mt.lock();
            cout << "1 Locking" << "\n"; 

        MessageData * d = connectClientSocket.receive(TIMEOUT_OFF);
        this->mt.unlock();
        if (d->type == TYPE_MAKE_CONNECTION) {
            thread listenToClientThread(&Server::listenToClient, this);
            listenToClientThread.detach();
        }
    }
}

int Server::getAvailablePort(){
    //TEM QUE COLOCAR MUTEX AQUI
    for(int i = 4001; i < 6000; i++){
        if(portsAvailable[i]){
            portsAvailable[i] = false;
            return i;
        }
    }
    return -1;
}

void Server::listenToClient()
{   
    int newPort = getAvailablePort();
    Packet packet = Dropbox::Packet(to_string(newPort));
	cout << "Creating socket at port" << to_string(newPort) << "\n"; 
    this->mt.lock();
    cout << "Locking" << "\n"; 

	WrapperSocket socket(newPort);
    if (this->clients.size() > 0)
	    connectClientSocket.send(packet);
    this->mt.unlock();
	while(1){
		socket.receive(TIMEOUT_OFF);
	}
}