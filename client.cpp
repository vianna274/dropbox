#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "src/WrapperSocket.hpp"
#include "src/Packet.hpp"
#include "src/constants.hpp"
#include "src/Client.hpp"

#define PORT 4000

using namespace std;

vector<string> getArguments(){
	string input;
	cout << "> ";
	getline(cin, input);
	input.erase(std::remove(input.begin(), input.end(), '\n'), input.end());
	vector<string> arguments;
	boost::split(arguments, input, [](char c){return c == ' ';});
	return arguments;
}

int main(int argc, char *argv[])
{
	bool endSession = false;
	
	Dropbox::Packet packet = Dropbox::Packet("Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
	Dropbox::WrapperSocket socket = Dropbox::WrapperSocket(SERVER_ADDR, SERVER_PORT);
	Dropbox::Client client("testUser", socket);

	while(!endSession){
		
		vector<string> arguments = getArguments();
		if(arguments.size() > 0 && arguments.size() < 3){
			string command = arguments[0];
			boost::to_upper(command);
			if (command == "UPLOAD"){
				if(arguments.size() != 2) puts("Wrong size of command -- upload <file path>");
				else client.upload(arguments[1]);
			}else if (command == "DOWNLOAD"){
				if(arguments.size() != 2) puts("Wrong size of command -- download <file path>");
				else client.download(arguments[1]);
			}else if (command == "DELETE"){
				if(arguments.size() != 2) puts("Wrong size of command -- delete <file path>");
				else client.download(arguments[1]);
			}else if (command == "LIST_SERVER"){
				if(arguments.size() != 1) puts("Wrong size of command -- list_server");
				else client.list_server();
			}else if (command == "LIST_CLIENT"){
				if(arguments.size() != 1) puts("Wrong size of command -- list_client");
				else client.list_client();
			}else if (command == "GET_SYNC_DIR"){
				if(arguments.size() != 1) puts("Wrong size of command -- get_sync_dir");
				else client.get_sync_dir();
			}else if(command == "EXIT"){
				if(arguments.size() != 1) puts("Wrong size of command -- exit");
				else client.exit();
				endSession = true;
			} else {
				puts("unidentified command!");
			}
		}
	}

	// printf("Enter the message: ");
	// bzero(buffer, 256);
	// fgets(buffer, 256, stdin);
	puts("Enviando");
	socket.send(packet);
	puts("Done");

	// n = sendto(sockfd, buffer, strlen(buffer), 0, (const struct sockaddr *) &serv_addr, sizeof(struct sockaddr_in));
	// if (n < 0)
	// 	printf("ERROR sendto");

	// length = sizeof(struct sockaddr_in);
	// n = recvfrom(sockfd, buffer, 256, 0, (struct sockaddr *) &from, &length);
	// if (n < 0)
	// 	printf("ERROR recvfrom");

	// printf("Got an ack: %s\n", buffer);

	// close(sockfd);
	return 0;
}
