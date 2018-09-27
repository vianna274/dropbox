#include "../include/Server.hpp"

using namespace std;
using namespace Dropbox;

Server::Server() : connectClientSocket(SERVER_PORT)
{
    initializePorts();

    while(true){
        this->connectNewClientMutex.lock();
        connectNewClient();
        this->connectNewClientMutex.unlock();
    }
}

void Server::initializePorts()
{
    for(int p = FIRST_PORT; p <= LAST_PORT; p++){
        portsAvailable[p - FIRST_PORT] = true;
    }
}

int Server::getAvailablePort()
{
    for(int p = FIRST_PORT; p <= LAST_PORT; p++){
        int i = p - FIRST_PORT;
        if(portsAvailable[i]){
            portsAvailable[i] = false;
            return p;
        }
    }
    portsMutex.unlock();
    return -1;
}

User* Server::getUser(string username)
{
    for(unsigned i = 0; i < users.size(); i++){
        if(users[i]->getUsername() == username){
            return users[i];
        }
    }
    return nullptr;
}

void Server::connectNewClient()
{

    MessageData *d = connectClientSocket.receive(TIMEOUT_OFF);
    
    if (d->type == TYPE_MAKE_CONNECTION) {
        string username(d->payload);
        User *user = getUser(username);
        if(user == nullptr){
            user = new User(username);
        }
        else if(user->getNumDevicesConnected() == MAX_DEVICES){
            cout << "NO MORE DEVICES AVAILABLE" << endl;
            refuseOverLimitClient(*user);
            return;
        }

        int newPort = getAvailablePort();
        MessageData packet; 
        strcpy(packet.payload, to_string(newPort).c_str());
        packet.type = TYPE_MAKE_CONNECTION; packet.totalSize = 1; packet.seq = 1;

	    WrapperSocket *socket = new WrapperSocket(newPort);
        connectClientSocket.send(packet);

        user->addDevice(socket);
        if(getUser(username) == nullptr){
            users.push_back(user);
        }

        thread listenToClientThread(&Server::listenToClient, this, socket);
        listenToClientThread.detach();
    }

}

void Server::refuseOverLimitClient(User user)
{
    MessageData packet;
    string message;
    message = "Number of devices for user " + user.getUsername() + " were used up! Max number of devices : " + to_string(MAX_DEVICES);
    strcpy(packet.payload, message.c_str());
    packet.type = TYPE_REJECT_TO_LISTEN; packet.totalSize = 1; packet.seq = 1;
    connectClientSocket.send(packet);
}

void Server::listenToClient(WrapperSocket *socket)
{   
	while(true){
		MessageData *data = socket->receive(TIMEOUT_OFF);
        switch(data->type){
            case TYPE_DATA:
            break;

            case TYPE_SEND_FILE:
            break;
        }
	}

    delete socket;
}