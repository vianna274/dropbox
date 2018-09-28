#include "../include/Server.hpp"

using namespace std;
using namespace Dropbox; 

Server::Server() : connectClientSocket(SERVER_PORT)
{
    cout << to_string(sizeof(FileRecord)) << endl;
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

vector<FileRecord> Server::getServerFileStatus(User *user)
{
    vector<FileRecord> files;
    struct stat filestatus;
    string extension;
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (user->getDirPath().c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type == 0x8) {
                stat((user->getDirPath() + ent->d_name).c_str(), &filestatus);
                extension = string(ent->d_name).substr(string(ent->d_name).find_last_of(".") + 1);
                FileRecord fileRecord = make_record(ent->d_name, extension.c_str(), ctime(&(filestatus.st_mtim.tv_sec)), filestatus.st_size);
                files.push_back(fileRecord);
            }
        }
        closedir(dir);
    }
    return files;
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

        vector<FileRecord> tmp = getServerFileStatus(user);

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
        switch(data->type){
            case TYPE_DATA:
                break;
            case TYPE_LIST_SERVER:{
                vector<FileRecord> files = getServerFileStatus(user);
                int seq = 0;
                for(FileRecord record : files) {
                    MessageData packet = make_packet(TYPE_DATA, seq, files.size()-1, sizeof(FileRecord), (char*)&record);
                    //FileRecord test = *((FileRecord*)(packet.payload));
                    //cout << test.filename << endl;
                    socket->send(&packet);
                    seq++;
                }
                break;
            }
            case TYPE_SEND_FILE:
                receiveUpload(string(data->payload), socket, user);
                break;

            case EXIT:
                int port = socket->getPortInt();
                user->closeDeviceSession(socket);
                setPortAvailable(port);
                exit = true;
                cout << "User " + user->getUsername() + " ended session on device on port " << port << endl;
                break;
        }
	}

    delete socket;
}

void Server::setPortAvailable(int port){
    this->portsAvailable[port - FIRST_PORT] = true;
}

void Server::receiveUpload(string filename, WrapperSocket *socket, User *user){
    string filePath = user->getDirPath() + filename;

    ofstream newFile;
	newFile.open(filePath, ofstream::trunc | ofstream::binary);
	if(!newFile.is_open()) {
		cout << "Erro ao receber arquivo " << filePath << ". Não foi possível criar cópia local." << endl;
		return;
    }
    int seqNumber, totalPackets;
    do{
        MessageData *packet = socket->receive(TIMEOUT_OFF);
        seqNumber = packet->seq;
        totalPackets = packet->totalSize;
        cout << "i-"<< seqNumber << " t-"<<totalPackets << " l-" <<packet->len << endl;
        newFile.write(packet->payload, packet->len);
    }while(seqNumber != totalPackets);

    newFile.close();

    //TODO UPLOAD USER !!!
}
