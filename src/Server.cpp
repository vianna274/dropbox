#include "../include/Server.hpp"

using namespace std;
using namespace Dropbox;

Server::Server() : connectClientSocket(SERVER_PORT){
    initializePorts();

    while(true){
        this->connectNewClientMutex.lock();
        connectNewClient();
        this->connectNewClientMutex.unlock();
    }
}

void Server::initializePorts(){
    for(int p = FIRST_PORT; p <= LAST_PORT; p++){
        portsAvailable[p - FIRST_PORT] = true;
    }
}

int Server::getAvailablePort(){
    portsMutex.lock();
    for(int p = FIRST_PORT; p <= LAST_PORT; p++){
        int i = p - FIRST_PORT;
        if(portsAvailable[i]){
            portsAvailable[i] = false;
            portsMutex.unlock();
            return p;
        }
    }
    portsMutex.unlock();
    return -1;
}

void Server::connectNewClient(){
    MessageData *d = connectClientSocket.receive(TIMEOUT_OFF);
    if (d->type == TYPE_MAKE_CONNECTION) {
        int newPort = getAvailablePort();
        MessageData packet; 
        strcpy(packet.payload, to_string(newPort).c_str());
        packet.type = TYPE_MAKE_CONNECTION;

	    WrapperSocket *socket = new WrapperSocket(newPort);
        connectClientSocket.send(packet);

        //CRIAR GRUPO COM O CLIENT AQUI

        thread listenToClientThread(&Server::listenToClient, this, socket);
        listenToClientThread.detach();
    }

}

void Server::listenToClient(WrapperSocket *socket)
{   
	while(true){
		MessageData *data = socket->receive(TIMEOUT_OFF);
        //TRATA O QUE O CLIENTE MANDOU
	}

    delete socket;
}