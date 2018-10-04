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
        closedir(dir);
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
            user = new User(username, rootDir+username+"/");
            users.push_back(user);
        }
        else if(user->getNumDevicesConnected() == MAX_DEVICES){
            refuseOverLimitClient(user);
            return;
        }

        int newPort = getAvailablePort();
        MessageData packet = make_packet(TYPE_MAKE_CONNECTION, 1, 1, -1, to_string(newPort).c_str());
	    WrapperSocket *socket = new WrapperSocket(newPort);
        connectClientSocket.send(&packet);

        user->addDevice(socket);

        cout << "User " << user->getUsername() << " connected on port " << newPort << ". " << "Device " << user->getNumDevicesConnected() << "/" << MAX_DEVICES << endl; 

        thread listenToClientThread(&Server::listenToClient, this, socket, user);
        listenToClientThread.detach();
    }
}

void Server::refuseOverLimitClient(User *user)
{
    string message = "Number of devices for user " + user->getUsername() + " were used up! Max number of devices : " + to_string(MAX_DEVICES);
    MessageData packet = make_packet(TYPE_REJECT_TO_LISTEN, 1, 1, -1, message.c_str());
    connectClientSocket.send(&packet);
}

void Server::listenToClient(WrapperSocket *socket, User *user)
{   
    bool exit = false;
	while(!exit){
		MessageData *data = socket->receive(TIMEOUT_OFF);
        user->lockDevices();
        switch(data->type){
            case TYPE_DATA:
                break;
            case TYPE_REQUEST_DOWNLOAD:
                sendFile(socket, user->getDirPath() + string(data->payload));
                break;
            case TYPE_DELETE:
                deleteFile(string(data->payload) + user->getDirPath());
                break;
            case TYPE_LIST_SERVER:
                sendFileList(socket, user->getDirPath(), getFileList(user->getDirPath()));
                break;
            case TYPE_SEND_FILE:
                receiveFile(socket, string(data->payload), user->getDirPath());
                break;
            case TYPE_REQUEST_UPLOAD_ALL:
                sendUploadAll(socket, user->getDirPath(), getFileList(user->getDirPath()));
                break;
            case TYPE_REQUEST_UPDATE:
                receiveAskUpdate(socket, user);
                break;
            case EXIT:
                exitUser(socket, user);
                exit = true;
                break;
        }
        user->unlockDevices();
	}

    delete socket;
}


void Server::receiveAskUpdate(WrapperSocket * socket, User * user) {
    vector<FileRecord> clientFiles = this->receiveFileList(socket);
    this->sendDeleteAll(socket);
    this->sendUploadAll(socket, user->getDirPath(), this->getFileList(user->getDirPath()));
    MessageData packet = make_packet(TYPE_REQUEST_UPDATE_DONE, 1, 1, -1, "request_update_done");
	socket->send(&packet);
}

void Server::setPortAvailable(int port){
    this->portsAvailable[port - FIRST_PORT] = true;
}

void Server::exitUser(WrapperSocket *socket, User *user){
    int port = socket->getPortInt();
    user->closeDeviceSession(socket);
    setPortAvailable(port);
    cout << "User " + user->getUsername() + " ended session on device on port " << port << endl;
}
