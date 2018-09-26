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
#include "include/WrapperSocket.hpp"
#include "include/MessageData.hpp"
#include "include/constants.hpp"
#include "include/Client.hpp"

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
	
	Dropbox::Client client("testUser", SERVER_ADDR, SERVER_PORT);

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

	return 0;
}
