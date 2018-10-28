#include "../include/Operations.hpp"

using namespace std;
using namespace Dropbox;

Operations::Operations() {}

vector<FileRecord> Operations::getFileList(string dirPath) {
    vector<FileRecord> files;
    struct stat filestatus;
    DIR *dir;
    struct dirent *ent;

    if ((dir = opendir (dirPath.c_str())) != NULL) {
        while ((ent = readdir (dir)) != NULL) {
            if(ent->d_type == 0x8) {
				stat((dirPath + ent->d_name).c_str(), &filestatus);
                FileRecord fileRecord = make_record(ent->d_name, filestatus.st_ctim.tv_sec, filestatus.st_atim.tv_sec, filestatus.st_mtim.tv_sec, filestatus.st_size);
				files.push_back(fileRecord);
            }
        }
        closedir(dir);
    }
    return files;
}

void Operations::sendFileList(WrapperSocket *socket, vector<FileRecord> files){
    if(files.empty()){
        MessageData packet = make_packet(TYPE_NOTHING_TO_SEND, 1, 1, -1, "nothing_to_send");
        socket->send(&packet);
        return;
    }
    int seq = 1;
    for(FileRecord record : files) {
        MessageData packet = make_packet(TYPE_DATA, seq, files.size(), sizeof(FileRecord), (char*)&record);
        socket->send(&packet);
        seq++;
    }
}

void Operations::receiveFile(WrapperSocket *socket, string filename, string dirPath) {
    string filePath = dirPath + filename;

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
        newFile.write(packet->payload, packet->len);
    }while(seqNumber != totalPackets);
    
    newFile.close();
}

FileRecord Operations::getRecord(vector<FileRecord> files, string filename) {
	for(FileRecord file : files) {
		if (string(file.filename) == filename)
			return file;
	}
}

void Operations::sendFile(WrapperSocket *socket, string filePath, FileRecord fileRec){

	string filename = filePath.substr(filePath.find_last_of("/\\") + 1);
	struct stat buffer;   
  	if(stat(filePath.c_str(), &buffer) != 0){
		  cout << "Failed to send: File " << filePath << " does not exist." << endl;
		  MessageData failed = make_packet(TYPE_NOTHING_TO_SEND, 1, 1, -1, "");
		  socket->send(&failed);
		  return;
	}
	MessageData packet = make_packet(TYPE_SEND_FILE, 1, 1, sizeof(FileRecord), (char *)&fileRec);
	socket->send(&packet);

	ifstream file;
	file.open(filePath, ifstream::in | ifstream::binary);
	file.seekg(0, file.end);
	int fileSize = file.tellg();
	file.seekg(0, file.beg);
	int totalPackets = 1 + ((fileSize - 1) / MESSAGE_LEN); // ceil(x/y)
	int lastPacketSize = fileSize % MESSAGE_LEN;
	int packetsSent = 0;
	int filePointer = 0;
	int packetSize = MESSAGE_LEN;
	char payload[MESSAGE_LEN];
	cout << "Uploading " << filePath << " Size: " << fileSize << " NumPackets: " << totalPackets << endl;

	while (packetsSent < totalPackets){
		file.seekg(filePointer, file.beg);
		filePointer += MESSAGE_LEN;
		if(packetsSent == totalPackets - 1) packetSize = lastPacketSize;
		file.read(payload, packetSize);
		MessageData packet = make_packet(TYPE_DATA, packetsSent + 1, totalPackets, packetSize, payload);
		socket->send(&packet);
		packetsSent++;
	}
	file.close();
}

void Operations::sendFile(WrapperSocket *socket, string filePath){

	string filename = filePath.substr(filePath.find_last_of("/\\") + 1);
	struct stat buffer;   
  	if(stat(filePath.c_str(), &buffer) != 0){
		  cout << "Failed to send: File " << filePath << " does not exist." << endl;
		  MessageData failed = make_packet(TYPE_NOTHING_TO_SEND, 1, 1, -1, "");
		  socket->send(&failed);
		  return;
	}
	MessageData packet = make_packet(TYPE_SEND_FILE_NO_RECORD, 1, 1, -1, filename.c_str());
	socket->send(&packet);

	ifstream file;
	file.open(filePath, ifstream::in | ifstream::binary);
	file.seekg(0, file.end);
	int fileSize = file.tellg();
	file.seekg(0, file.beg);
	int totalPackets = 1 + ((fileSize - 1) / MESSAGE_LEN); // ceil(x/y)
	int lastPacketSize = fileSize % MESSAGE_LEN;
	int packetsSent = 0;
	int filePointer = 0;
	int packetSize = MESSAGE_LEN;
	char payload[MESSAGE_LEN];
	cout << "Uploading " << filePath << " Size: " << fileSize << " NumPackets: " << totalPackets << endl;

	while (packetsSent < totalPackets){
		file.seekg(filePointer, file.beg);
		filePointer += MESSAGE_LEN;
		if(packetsSent == totalPackets - 1) packetSize = lastPacketSize;
		file.read(payload, packetSize);
		MessageData packet = make_packet(TYPE_DATA, packetsSent + 1, totalPackets, packetSize, payload);
		socket->send(&packet);
		packetsSent++;
	}
	file.close();
}

FileRecord Operations::sendFileClient(WrapperSocket *socket, string filePath){

	string filename = filePath.substr(filePath.find_last_of("/\\") + 1);
	struct stat buffer;   
  	if(stat(filePath.c_str(), &buffer) != 0){
		  cout << "Failed to send: File " << filePath << " does not exist." << endl;
		  MessageData failed = make_packet(TYPE_NOTHING_TO_SEND, 1, 1, -1, "");
		  socket->send(&failed);
		  exit(-1);
	}
	MessageData packet = make_packet(TYPE_SEND_FILE_NO_RECORD, 1, 1, -1, filename.c_str());
	socket->send(&packet);

	ifstream file;
	file.open(filePath, ifstream::in | ifstream::binary);
	file.seekg(0, file.end);
	int fileSize = file.tellg();
	file.seekg(0, file.beg);
	int totalPackets = 1 + ((fileSize - 1) / MESSAGE_LEN); // ceil(x/y)
	int lastPacketSize = fileSize % MESSAGE_LEN;
	int packetsSent = 0;
	int filePointer = 0;
	int packetSize = MESSAGE_LEN;
	char payload[MESSAGE_LEN];
	cout << "Uploading " << filePath << " Size: " << fileSize << " NumPackets: " << totalPackets << endl;

	while (packetsSent < totalPackets){
		file.seekg(filePointer, file.beg);
		filePointer += MESSAGE_LEN;
		if(packetsSent == totalPackets - 1) packetSize = lastPacketSize;
		file.read(payload, packetSize);
		MessageData packet = make_packet(TYPE_DATA, packetsSent + 1, totalPackets, packetSize, payload);
		socket->send(&packet);
		packetsSent++;
	}
	file.close();

	return this->receiveFileRecord(socket);
}

FileRecord Operations::receiveFileRecord(WrapperSocket * socket) {
	MessageData * message = socket->receive(TIMEOUT_OFF);
	FileRecord * fileRecord = (FileRecord*)message->payload;
	return *fileRecord;
}

vector<FileRecord> Operations::receiveFileList(WrapperSocket * socket) {
	MessageData *unconvertedFiles;
	FileRecord record;
	vector<FileRecord> files;
	do {
		unconvertedFiles = socket->receive(TIMEOUT_OFF);
		if(unconvertedFiles->type == TYPE_NOTHING_TO_SEND) break;
		record = *((FileRecord*)unconvertedFiles->payload);
		files.push_back(record);
	} while(unconvertedFiles->seq != unconvertedFiles->totalSize);
	return files;
}

void Operations::printFileList(vector<FileRecord> fileRecords) {
	for(FileRecord record : fileRecords) {
		cout << setw(10);
		cout << record.filename << " changed time: " << ctime(&record.creationTime) << " 	   last modified: " << ctime(&record.modificationTime) << " 	   last accessed: " << ctime(&record.accessTime)<< endl;
	}
}

void Operations::receiveUploadAll(WrapperSocket * socket, string dirPath) {
	while(1) {
		MessageData * packet = socket->receive(TIMEOUT_OFF);
		if (packet->type == TYPE_SEND_UPLOAD_ALL_DONE ||
			packet->type == TYPE_NOTHING_TO_SEND)
			break;
		this->receiveFile(socket, string(packet->payload), dirPath);
	}
}

void Operations::sendUploadAll(WrapperSocket * socket, string dirPath, vector<FileRecord> files) {
	if(files.empty()) {
		this->sendNothing(socket);
        return;
    }
	MessageData packet = make_packet(TYPE_SEND_UPLOAD_ALL, 1, 1, -1, "send_upload_all");
	socket->send(&packet);
    int seq = 1;
    for(FileRecord record : files) {
		this->sendFile(socket, dirPath + record.filename, record);
        seq++;
    }
	packet = make_packet(TYPE_SEND_UPLOAD_ALL_DONE, 1, 1, -1, "send_upload_all_done");
	socket->send(&packet);
}

void Operations::sendNothing(WrapperSocket * socket) {
	MessageData packet = make_packet(TYPE_NOTHING_TO_SEND, 1, 1, -1, "nothing_to_send");
	socket->send(&packet);
}

void Operations::sendDeleteFile(WrapperSocket * socket, string filename){
	MessageData request = make_packet(TYPE_DELETE, 1, 1, -1, filename.c_str());
	socket->send(&request);
	cout << "Deleting " << filename << endl;

}

void Operations::sendDeleteAll(WrapperSocket * socket) {
	MessageData packet = make_packet(TYPE_DELETE_ALL, 1, 1, -1, "delete_all");
	socket->send(&packet);
}

void Operations::deleteFile(string filepath) {
	if(remove(filepath.c_str()) != 0 )
		cout << "Error deleting file: " << filepath << endl;
}

void Operations::deleteAll(vector<FileRecord> files, string dirPath) {
	for(FileRecord file : files) {
		this->deleteFile(dirPath + file.filename);
	}
}