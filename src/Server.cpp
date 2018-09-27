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
                User *user = new User(ent->d_name);
                users.push_back(user);
            }
        }
        closedir (dir);
    } else if(mkdir(rootDir.c_str(), 0777) == 0) cout << "CREATED ROOT DIR" << endl;
    else cout << "COULDN'T OPEN OR CREATE ROOT DIR" << endl;

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
    string userDir = rootDir + username;
    cout << userDir << endl;
    if(mkdir(userDir.c_str(), 0777) == 0) cout << "CREATED USER DIR" << endl;
    else cout << "COULDN'T CREATE USERS DIRECTORY!" << endl;
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