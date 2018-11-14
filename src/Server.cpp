#include "../include/Server.hpp"

using namespace std;
using namespace Dropbox; 

Server::Server(string ipLocal) : connectClientSocket(SERVER_PORT), listenToServersSocket(BACKUPS_PORT), talkToPrimary()
{
    this->isMain = true;
    this->ipLocal = ipLocal;
    this->ipMain = ipLocal;
    this->backups = {};
    this->usersIp = {};
    this->backupsSockets = {};
    initializeUsers();
    initializePorts();
}

Server::Server(string ipLocal, string ipMain, vector<string> backups) : connectClientSocket(SERVER_PORT), listenToServersSocket(BACKUPS_PORT), talkToPrimary(ipMain, BACKUPS_PORT)
{
    this->isMain = false;
    this->ipLocal = ipLocal;
    this->ipMain = ipMain;
    this->backups = backups;
    this->usersIp = {};
    this->backupsSockets = {};
    initializeUsers();
    initializePorts();
}

void Server::run(){

    thread listenToServersThread(&Server::listenToServers, this);
    listenToServersThread.detach();

    if(!this->isMain) {
        this->makeConnection();
    }
    while(true){
        if(this->isMain){
            connectNewClient();
        }
        else{
            MessageData packet = make_packet(TYPE_PING, 1, 1, -1, "");
            bool isPrimaryAlive = this->talkToPrimary.send(&packet);
            cout << "Server ta: " << isPrimaryAlive << endl;
            if(!isPrimaryAlive){
                this->isMain = true;
                this->propagateNewBoss();
            }
            this_thread::sleep_for(chrono::milliseconds(3000));
        }
    }
}

Server::~Server(){
    for(User *user : users){
        delete user;
    }
}

void Server::propagateNewBoss() {
    for (string ip : this->usersIp) {
        WrapperSocket userSocket(ip, 10000);
        MessageData packet = make_packet(TYPE_NEW_BOSS, 1, 1, -1, this->ipLocal.c_str());
        userSocket.send(&packet);
    }
}

void Server::makeConnection(){
    MessageData packet = make_packet(TYPE_MAKE_BACKUP, 1, 1, -1, ipLocal.c_str());

    talkToPrimary.send(&packet);
}

void Server::answer(string ip) {
    WrapperSocket socket(ip, 9000);
    MessageData packet = make_packet(TYPE_ANSWER, 1, 1, ip.size(), ip.c_str());
    socket.send(&packet);
}

void Server::becomeMain() {
    this->status = STATUS_COORDINATOR;
    for(string backupsIp: ip) {
        WrapperSocket socket(ip, 9000);
        MessageData packet = make_packet(TYPE_COORDINATOR, 1, 1, ipLocal.size(), ipLocal.c_str());
        socket->send(&packet);
    }
    this->isMain = true;
    this->propagateNewBoss();
}

vector<string> Server::getHighers() {
    vector<string> highers;
    
    for(string ip : backupIps) {
        if (ip > ipLocal)
            highers.push_back(ip);
    }
    return highers;
}

void Server::sendHighersElection(vector<string> highers) {
    for(string higher : highers) {
        WrapperSocket socket(higher);
        MessageData packet = make_packet(TYPE_ELECTION, 1, 1, higher.size(), higher);
        socket->send(&packet);
    }
}

void Server::listenToServers(){
    string ipDoBackup;
    WrapperSocket *talkToBackup;
    while(true){
        MessageData *data = listenToServersSocket.receive(TIMEOUT_OFF);
        string username = string(data->username);
        User * user = getUser(username);
        string ip;
        cout << "Received " << string(data->payload) << " " << data->type << endl;
        FileRecord fileRecord;
        switch (data->type)
        {
            case TYPE_COORDINATOR:
                this->status = STATUS_NORMAL;
                break;
            case TYPE_ANSWER:
                this->status = STATUS_WAITING_COORDINATOR;
                // TIMEOUT ??
                break;
            case TYPE_ELECTION:
                ip = string(data->payload);
                this->answer(ip);
                if (this->status == STATUS_NORMAL) {
                    this->status = STATUS_ELECTION;
                    if (this->getHighers().size() > 0)
                        this->sendHighersElection();
                    else
                        this->becomeMain();
                }
            case TYPE_MAKE_BACKUP:
                cout << "MAKE" << endl;
                ipDoBackup = string(data->payload);
                talkToBackup = new WrapperSocket(ipDoBackup, BACKUPS_PORT);
                this->backupsSockets.push_back(talkToBackup);
                break;
            case TYPE_PING:
                cout << "PINGING" << endl;
                break;
            case TYPE_CREATE_USER:
                cout << "CREATING USER with " << string(data->payload) << endl;
                this->usersIp.push_back(string(data->payload));
                if(user == nullptr) {
                    cout << "Pushing new user" << endl;
                    user = new User(username, rootDir+username+"/");
                    users.push_back(user);
                }
                break;
            case TYPE_DELETE: 
                cout << "DELETING" << endl;
                deleteFile(user->getDirPath() + string(data->payload));
                user->removeFileRecord(string(data->payload));
                break;
            case TYPE_SEND_FILE:
                cout << "CREATING FILE" << endl;
                fileRecord = *((FileRecord *)data->payload);
                receiveFile(&listenToServersSocket, string(data->payload), user->getDirPath());
                user->updateFileRecord(fileRecord);
                break;
            //case de propagar coisas 
            default:
                cout << "PACOTE INCORRETO! " << data->type << endl;
                break;
        }
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
        string username(d->username);
        string userIp(d->payload);
        User *user = getUser(username);
        if(user == nullptr){
            user = new User(username, rootDir+username+"/");
            users.push_back(user);
        }
        else{
            user->lockDevices();
            if(user->getNumDevicesConnected() == MAX_DEVICES){
                refuseOverLimitClient(user);
                user->unlockDevices();
                return;
            }
            user->unlockDevices();
        }
        int newPort = getAvailablePort();
        MessageData packet = make_packet(TYPE_MAKE_CONNECTION, 1, 1, -1, to_string(newPort).c_str());
	    WrapperSocket *socket = new WrapperSocket(newPort);
        connectClientSocket.send(&packet);
        
        user->lockDevices();
        user->addDevice(socket);
        user->unlockDevices();

        cout << "User " << user->getUsername() << " connected on port " << newPort << ". " << "Device " << user->getNumDevicesConnected() << "/" << MAX_DEVICES << endl; 
        this->propagateConnection(username, userIp);
        thread listenToClientThread(&Server::listenToClient, this, socket, user);
        listenToClientThread.detach();
    }
}

void Server::propagateDelete(string filename, string username) 
{
    User * user = getUser(username);
    for(WrapperSocket * socket : this->backupsSockets) {
        this->sendDeleteFile(socket, filename.c_str(), username);
        user->removeFileRecord(filename);
    }
}

void Server::propagateFile(string filename, string username) 
{
    User * user = getUser(username);
    for(WrapperSocket * socket : this->backupsSockets) {
        FileRecord record = this->getRecord(user->getFileRecords(),filename);
        this->sendFile(socket, user->getDirPath() + filename, record, username);
    }
}

void Server::propagateConnection(string username, string userIp)
{
    MessageData packet = make_packet(TYPE_CREATE_USER, 1, 1, -1, userIp.c_str(), username.c_str());
    for(WrapperSocket * socket : this->backupsSockets) {
        socket->send(&packet);
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
    FileRecord * temp = NULL;
    FileRecord fileTemp;
    vector<FileRecord> tempFiles = user->getFileRecords();
	while(!exit){
		MessageData *data = socket->receive(TIMEOUT_OFF);
        user->lockDevices();
        switch(data->type){
            case TYPE_REQUEST_DOWNLOAD:
                tempFiles = user->getFileRecords();
                fileTemp = this->getRecord(tempFiles, string(data->payload));
                sendFile(socket, user->getDirPath() + string(data->payload), fileTemp, string(data->username));
                break;
            case TYPE_DELETE: 
                deleteFile(user->getDirPath() + string(data->payload));
                user->removeFileRecord(string(data->payload));
                propagateDelete(string(data->payload), string(data->username));
                break;
            case TYPE_LIST_SERVER:
                sendFileList(socket, user->getFileRecords());
                break;
            case TYPE_SEND_FILE_NO_RECORD:
                receiveFile(socket, string(data->payload), user->getDirPath());
                user->updateFileRecord(this->getRecord(this->getFileList(user->getDirPath()), string(data->payload)));
                sendFileRecord(socket, string(data->payload), user);
                this->propagateFile(string(data->payload), string(data->username));
                break;
            case TYPE_REQUEST_UPLOAD_ALL:
                sendUploadAll(socket, user->getDirPath(), getFileList(user->getDirPath()), user->getUsername());
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

void Server::sendFileRecord(WrapperSocket * socket, string filename, User * user) {
    FileRecord record = this->getRecord(user->getFileRecords(), filename);
    MessageData packet = make_packet(TYPE_DATA, 1, 1, sizeof(FileRecord), (char*)&record);
    socket->send(&packet);
}

int Server::lookForRecordAndRemove(FileRecord file, vector<FileRecord> *files, FileRecord *updatedFile) {
    vector<FileRecord>::iterator it;
    for(it = files->begin(); it != files->end(); it++) {
        if (string(it->filename) == string(file.filename)) {
            if (it->modificationTime == file.modificationTime) {
                files->erase(it);
                return OK;
            }
            else {
                *updatedFile = *it;
                files->erase(it);
                return UPDATE;
            }
        }
    }
    return DELETE;
}

void Server::updateClient(vector<FileRecord> serverFiles, vector<FileRecord> clientFiles, WrapperSocket *socket, User *user) {
    cout << "Checking if user " << user->getUsername() << " is updated." << endl;
    vector<FileRecord>::iterator it;
    FileRecord temp;
    for(it = clientFiles.begin(); it != clientFiles.end(); it++) {
        switch(this->lookForRecordAndRemove(*it, &serverFiles, &temp)) {
            case OK: 
                break;
            case DELETE:
                cout << "Deleting " <<  string(it->filename) << " from " << user->getUsername() << " local device." << endl;
                this->sendDeleteFile(socket, string(it->filename));
                break;
            case UPDATE:
                cout << "Updating " << temp.filename << " " << ctime(&temp.modificationTime) << endl; 
                this->sendDeleteFile(socket, string(temp.filename));
                this->sendFile(socket, user->getDirPath() + string(temp.filename), temp, user->getUsername());
                break;
        }
    }
    // Send files that the client didn't have at the moment
    for(FileRecord serverFile: serverFiles) {
        cout << "Sending " << serverFile.filename << " " << serverFile.modificationTime << endl; 
        this->sendFile(socket, user->getDirPath() + string(serverFile.filename), serverFile, user->getUsername());
    }

}

void Server::receiveAskUpdate(WrapperSocket * socket, User * user) {
    vector<FileRecord> clientFiles = this->receiveFileList(socket);
    this->updateClient(this->getFileList(user->getDirPath()), clientFiles, socket, user);
    MessageData packet = make_packet(TYPE_REQUEST_UPDATE_DONE, 1, 1, -1, "request_update_done");
	socket->send(&packet);
}

void Server::setPortAvailable(int port){
    portsMutex.lock();
    this->portsAvailable[port - FIRST_PORT] = true;
    portsMutex.unlock();
}

void Server::exitUser(WrapperSocket *socket, User *user){
    int port = socket->getPortInt();
    user->closeDeviceSession(socket);
    setPortAvailable(port);
    cout << "User " + user->getUsername() + " ended session on device on port " << port << endl;
}
