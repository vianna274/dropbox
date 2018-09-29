#include "../include/Operations.hpp"

using namespace std;
using namespace Dropbox;

Operations::Operations() {}

void Operations::sendFileList(WrapperSocket *socket, string dirPath, vector<FileRecord> files){
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

void Operations::receiveUpload(WrapperSocket * socket, string filename, string dirPath) {
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
    
    cout << "Received file " << filename << "." << endl;
    newFile.close();
}

void Operations::sendUpload(WrapperSocket * socket, string filePath){

	string filename = filePath.substr(filePath.find_last_of("/\\") + 1);
	struct stat buffer;   
  	if(stat(filePath.c_str(), &buffer) != 0){
		  cout << "Failed to upload: File " << filePath << " does not exist." << endl;
		  return;
	}
	MessageData packet = make_packet(TYPE_SEND_FILE, 1, 1, -1, filename.c_str());
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

void Operations::receiveFileList(WrapperSocket * socket) {
	cout << "Files on server:" << endl;
	MessageData packet = make_packet(TYPE_LIST_SERVER, 1, 1, -1, "list_server");
	MessageData *unconvertedFiles;
	FileRecord record;
	socket->send(&packet);
	do {
		unconvertedFiles = socket->receive(TIMEOUT_OFF);
		if(unconvertedFiles->type == TYPE_NOTHING_TO_SEND) break;
		record = *((FileRecord*)unconvertedFiles->payload);
		cout << setw(10);
		cout << record.filename << " 	type: " << record.type << " 	last modified: " << record.date << endl;
	} while(unconvertedFiles->seq != unconvertedFiles->totalSize);
}

void Operations::receiveUploadAll(WrapperSocket * socket, string dirPath) {
	MessageData request = make_packet(TYPE_REQUEST_UPLOAD_ALL, 1, 1, -1, "nothing_to_send");
	socket->send(&request);
	MessageData *packet = socket->receive(TIMEOUT_OFF);
	if (packet->type != TYPE_SEND_UPLOAD_ALL) 
		return;
	while(1) {
		packet = socket->receive(TIMEOUT_OFF);
		if (packet->type == TYPE_SEND_UPLOAD_ALL_DONE)
			break;
		this->receiveUpload(socket, string(packet->payload), dirPath + '/');
	}
}

void Operations::sendUploadAll(WrapperSocket * socket, string dirPath, vector<FileRecord> files) {
	if(files.empty()){
		this->sendNothing(socket);
        return;
    }
	MessageData packet = make_packet(TYPE_SEND_UPLOAD_ALL, 1, 1, -1, "nothing_to_send");
	socket->send(&packet);
    int seq = 1;
    for(FileRecord record : files) {
		this->sendUpload(socket, dirPath + '/' + record.filename);
        seq++;
    }
	packet = make_packet(TYPE_SEND_UPLOAD_ALL_DONE, 1, 1, -1, "nothing_to_send");
	socket->send(&packet);
}

void Operations::sendNothing(WrapperSocket * socket) {
	MessageData packet = make_packet(TYPE_NOTHING_TO_SEND, 1, 1, -1, "nothing_to_send");
	socket->send(&packet);
}