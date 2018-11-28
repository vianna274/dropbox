#include "../include/Server.hpp"

using namespace std;
using namespace Dropbox; 

Server::Server(string ipLocal) : connectClientSocket(SERVER_PORT), listenToServersSocket(BACKUPS_PORT)
{
    this->isMain = true;
    this->ipLocal = ipLocal;
    this->ipMain = ipLocal;
    this->backups = {};
    this->usersIp = {};
    this->backupsSockets = {};
    this->status = STATUS_NORMAL;
    initializeUsers();
    initializePorts();
}

Server::Server(string ipLocal, string ipMain, vector<string> backups) : connectClientSocket(SERVER_PORT), listenToServersSocket(BACKUPS_PORT)
{
    this->isMain = false;
    this->ipLocal = ipLocal;
    this->ipMain = ipMain;
    this->backups = backups;
    this->usersIp = {};
    this->backupsSockets = {};
    this->status = STATUS_NOT_YET;
    this->talkToPrimary = new WrapperSocket(ipMain, BACKUPS_PORT);
    initializeUsers();
    initializePorts();
}

void Server::run(){

    thread listenToServersThread(&Server::listenToServers, this, &(this->listenToServersSocket));
    listenToServersThread.detach();

    if(!this->isMain) {
        this->makeConnection();
    }
    while(true){
        if(this->isMain){
            connectNewClient();
        }
        else {
            this->electionMutex.lock();
            if (this->status == STATUS_NORMAL) {
                this->electionMutex.unlock();
                MessageData packet = make_packet(TYPE_PING, 1, 1, -1, "");
                talkToPrimaryMtx.lock();
                bool isPrimaryAlive = this->talkToPrimary->send(&packet, 100);
                talkToPrimaryMtx.unlock();
                this->electionMutex.lock();
                if(!isPrimaryAlive && this->status == STATUS_NORMAL){
                    this->status = BEGIN_ELECTION;
                    this->electionMutex.unlock();
                    backupList.lock();
                    this->removeFromBackup(this->backups, this->ipMain);
                    backupList.unlock();
                    cout << "Setou Begin_election" << endl;
                } else {
                    this->electionMutex.unlock();
                }
                this_thread::sleep_for(chrono::milliseconds(3000));
            } else {
                this->electionMutex.unlock();
            }
        }
        
    }
}

void Server::removeFromBackup(vector<string> backups, string main) {
    vector<string>::iterator it = backups.begin();
    vector<string> newBackups;
    for(it; it != backups.end(); it++) {
        cout << "trying to remove " << *it << " with " << main << endl;
        if (main.compare(*it) == 0) {
            cout << "removed" << endl;
            // it = backups.erase(it);
        } else {
            newBackups.push_back(*it);
        }
    }
    this->backups.clear();
    for(string backup: newBackups) {
        this->backups.push_back(backup);
    }
}

Server::~Server(){
    for(User *user : users){
        delete user;
    }
}

void Server::propagateNewBoss() {
    for (string ip : this->usersIp) {
        cout << "Propagando new Boss para " << ip << " com ip " << this->ipLocal << endl;
        WrapperSocket userSocket(ip, 10000);
        MessageData packet = make_packet(TYPE_NEW_BOSS, 1, 1, -1, this->ipLocal.c_str());
        userSocket.send(&packet);
    }
}

void Server::makeConnection(){
    MessageData packet = make_packet(TYPE_MAKE_BACKUP, 1, 1, -1, ipLocal.c_str());
    talkToPrimaryMtx.lock();
    talkToPrimary->send(&packet);
    talkToPrimaryMtx.unlock();
}

void Server::answer(string ip) {
    WrapperSocket socket(ip, 9000);
    cout << "Enviando answer para " << ip << endl;
    MessageData packet = make_packet(TYPE_ANSWER, 1, 1, ip.size(), ip.c_str());
    socket.send(&packet, 100);
}

void Server::becomeMain() {
    backupList.lock();
    for(string ip : this->backups) {
        WrapperSocket socket(ip, 9000);
        cout << "Enviando Coordinator" << endl;
        MessageData packet = make_packet(TYPE_COORDINATOR, 1, 1, ipLocal.size(), ipLocal.c_str());
        socket.send(&packet, 100);
    }
    backupList.unlock();
    this->isMain = true;
    this->electionMutex.lock();
    this->status = STATUS_COORDINATOR;
    this->electionMutex.unlock();
    this->propagateNewBoss();
}

vector<string> Server::getHighers() {
    vector<string> highers;
    backupList.lock();
    removeFromBackup(this->backups, this->ipMain);
    for(string ip : this->backups) {
        if (ip > ipLocal)
            highers.push_back(ip);
    }
    backupList.unlock();
    return highers;
}

void Server::sendHighersElection(vector<string> highers) {
    for(string higher : highers) {
        cout << "Enviando para " << higher << endl;
        WrapperSocket socket(higher, 9000);
        MessageData packet = make_packet(TYPE_ELECTION, 1, 1, ipLocal.size(), ipLocal.c_str());
        socket.send(&packet);
    }
}

void Server::startElection(vector<string> highers)
{
    this->electionMutex.lock();
    this->status = STATUS_ELECTION;
    this->electionMutex.unlock();
    cout << "Enviando Election para os maiores" << endl;
    this->sendHighersElection(highers);
    if (this->waitForAnswer()) {
        if (this->waitForCoordinator() || this->status == STATUS_NOT_YET)
            return;
        cout << "coordinator aqui" << endl;
        this->becomeMain();
    }
    cout << "coordinator aqui 2" << endl;
    this->becomeMain();
}

void Server::createNewPortBackup(WrapperSocket * socket)
{
    int newPort = getAvailablePort();
    MessageData packet = make_packet(TYPE_MAKE_CONNECTION, 1, 1, -1, to_string(newPort).c_str());
    WrapperSocket * newSocket = new WrapperSocket(newPort);
    socket->send(&packet);
    thread listenToBackupThread(&Server::listenToServers, this, newSocket);
    listenToBackupThread.detach();
}

void Server::handleReceiveCoordinator(MessageData * res) {
    this->electionMutex.lock();
    this->status = STATUS_NOT_YET;
    this->electionMutex.unlock();
    backupList.lock();
    removeFromBackup(this->backups, this->ipMain);
    backupList.unlock();
    cout << "Recebi Coordinator " << string(res->payload) << endl;
    this->ipMain = string(res->payload);
    talkToPrimaryMtx.lock();
    this->talkToPrimary = new WrapperSocket(ipMain, 9000);
    talkToPrimaryMtx.unlock();
    this->makeConnection();
    this->electionMutex.lock();
    this->status = STATUS_NORMAL;
    this->electionMutex.unlock();
}

void Server::listenToServers(WrapperSocket * socket){
    string ipDoBackup;
    WrapperSocket *talkToBackup;
    cout << "Listening to " << socket->getPortInt() << endl;
    MessageData * data = NULL;
    while(true){
        this->electionMutex.lock();
        if (this->status == BEGIN_ELECTION) {
            cout << "Entrou Begin_election" << endl;
            this->status = STATUS_ELECTION;
            this->electionMutex.unlock();
            if (this->getHighers().size() > 0) {
                cout << "comecei eleicao 2" << endl;
                this->startElection(this->getHighers());
            }
            else {
                cout << "coordinator aqui 5" << endl;
                this->becomeMain();
            }
        }
        else {
            this->electionMutex.unlock();
            data = socket->receive(TIMEOUT_ON, 5);
        }
        if (data == NULL)
            continue;
        string username = string(data->username);
        User * user = getUser(username);
        string ip;
        FileRecord fileRecord;
        switch (data->type)
        {
            case TYPE_MAKE_CONNECTION:
                cout << "Criando novo Socket Ip: " << ipMain << " Port: " << string(data->payload) << endl;
                talkToPrimaryMtx.lock();
                this->talkToPrimary = new WrapperSocket(ipMain, stoi(data->payload));
                talkToPrimaryMtx.unlock();
                this->electionMutex.lock();
                this->status = STATUS_NORMAL;
                this->electionMutex.unlock();
                break;
            case TYPE_COORDINATOR:
                handleReceiveCoordinator(data);
                break;
            case TYPE_ANSWER:
                cout << "Recebi Answer 2" << endl;
                if(!this->waitForCoordinator() && this->status == STATUS_ELECTION) {
                    cout << "coordinator aqui 3" << endl;
                    this->becomeMain();
                }
                break;
            case TYPE_ELECTION:
                cout << "Recebi Election do" << string(data->payload) << endl;
                ip = string(data->payload);
                this->answer(ip);
                this->electionMutex.lock();
                if (this->status == STATUS_NORMAL) {
                    this->status = STATUS_ELECTION;
                    this->electionMutex.unlock();
                    if (this->getHighers().size() > 0) {
                        cout << "comecei eleicao 1" << endl;
                        this->startElection(this->getHighers());
                    }
                    else {
                        cout << "coordinator aqui 4" << endl;
                        this->becomeMain();
                    }
                }
                this->electionMutex.unlock(); 
                break;
            case TYPE_MAKE_BACKUP:
                cout << "Enviando nova porta para o Backup" << endl;
                ipDoBackup = string(data->payload);
                talkToBackup = new WrapperSocket(ipDoBackup, BACKUPS_PORT);
                this->backupsSockets.push_back(talkToBackup);
                this->createNewPortBackup(talkToBackup);
                break;
            case TYPE_PING:
                //cout << "PINGING" << endl;
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
    cout << "Recebi algo na porta: " << connectClientSocket.getPortInt() << " tipo: " << d->type << endl;
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
    propagationMutex.lock();
    User * user = getUser(username);
    for(WrapperSocket * socket : this->backupsSockets) {
        this->sendDeleteFile(socket, filename.c_str(), username);
        user->removeFileRecord(filename);
    }
    propagationMutex.unlock();
}

void Server::propagateFile(string filename, string username) 
{
    propagationMutex.lock();
    User * user = getUser(username);
    for(WrapperSocket * socket : this->backupsSockets) {
        cout << "Propagating to: " << socket->getPortInt() << endl;
        FileRecord record = this->getRecord(user->getFileRecords(),filename);
        this->sendFile(socket, user->getDirPath() + filename, record, username);
    }
    propagationMutex.unlock();
}

void Server::propagateConnection(string username, string userIp)
{   
    propagationMutex.lock();
    MessageData packet = make_packet(TYPE_CREATE_USER, 1, 1, -1, userIp.c_str(), username.c_str());
    for(WrapperSocket * socket : this->backupsSockets) {
        socket->send(&packet);
    }
    propagationMutex.unlock();
}

void Server::refuseOverLimitClient(User *user)
{
    string message = "Number of devices for user " + user->getUsername() + " were used up! Max number of devices : " + to_string(MAX_DEVICES);
    MessageData packet = make_packet(TYPE_REJECT_TO_LISTEN, 1, 1, -1, message.c_str());
    connectClientSocket.send(&packet);
}

bool Server::waitForAnswer()
{
    while(true) {
        MessageData * res = this->listenToServersSocket.receive(TIMEOUT_ON, 1500);
        if (res == NULL)
            return false;
        cout << res->type << endl;
        if (res->type == TYPE_ANSWER) {
            cout << "Recebi answer 1" << endl;
            return true;
        }
        if (res->type == TYPE_COORDINATOR) {
            handleReceiveCoordinator(res);
            return true;
        }
    }
}

bool Server::waitForCoordinator()
{
    while(true) {
        MessageData * res = this->listenToServersSocket.receive(TIMEOUT_ON, 1500);
        if (res == NULL)
            return false;
        if (res->type == TYPE_COORDINATOR) {
            handleReceiveCoordinator(res);
            return true;
        }
    }
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
    MessageData packet = make_packet(TYPE_REQUEST_UPDATE_DONE, 1, 1, sizeof("request_update_done"), "request_update_done");
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
