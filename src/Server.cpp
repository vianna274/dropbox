#include "../include/Server.hpp"

using namespace std;
using namespace Dropbox;

Server::Server() : connectClientSocket(SERVER_PORT)
{
    initializeUsers();
    initializePorts();

    while(true){
        connectNewClient();
    }
}

void Server::initializeUsers()
{
    DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (rootDir.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type == 0x4 && string(ent->d_name) != string("..") && string(ent->d_name) != string(".")) {
                User *user = new User(ent->d_name, rootDir+ent->d_name+"/");
                users.push_back(user);
            }
        }
        closedir (dir);
    } 
    else if(mkdir(rootDir.c_str(), 0777) == 0) 
        cout << "CREATED ROOT DIR" << endl;
    else 
        cout << "COULDN'T OPEN OR CREATE ROOT DIR" << endl;

    cout << "System Users :" << endl;
    for(User *user : users)
    {
        cout << user->getUsername().c_str() << endl;
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
            cout << "User don't exist yet" << endl;
            user = new User(username, rootDir+username+"/");
            users.push_back(user);
        }
        else if(user->getNumDevicesConnected() == MAX_DEVICES){
            cout << "NO MORE DEVICES AVAILABLE" << endl;
            refuseOverLimitClient(*user);
            return;
        }
        cout<< "num devices"<< user->getNumDevicesConnected() << endl;

        int newPort = getAvailablePort();
        MessageData *packet = make_packet(TYPE_MAKE_CONNECTION, 1, 1, -1, to_string(newPort).c_str());
	    WrapperSocket *socket = new WrapperSocket(newPort);
        connectClientSocket.send(*packet);
        free(packet);

        user->addDevice(socket);

        thread listenToClientThread(&Server::listenToClient, this, socket);
        listenToClientThread.detach();
    }

}

void Server::refuseOverLimitClient(User user)
{
    string message = "Number of devices for user " + user.getUsername() + " were used up! Max number of devices : " + to_string(MAX_DEVICES);
    MessageData *packet = make_packet(TYPE_REJECT_TO_LISTEN, 1, 1, -1, message.c_str());

    connectClientSocket.send(*packet);
    free(packet);
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