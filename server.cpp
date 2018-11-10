#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include "include/WrapperSocket.hpp"
#include "include/constants.hpp"
#include "include/Server.hpp"

using namespace std;
using namespace Dropbox;

int main(int argc, char *argv[])
{	
	Server *server;
	if(argc == 2){
		// main
		string ipLocal(argv[1]);
		server = new Server(ipLocal);
	}
	else{
		// backups
		string ipLocal(argv[1]);
		string ipMain(argv[2]);
		vector<string> ipBackups;
		for(int i = 3; i<argc; i++){
			ipBackups.push_back(argv[i]);
		}
		server = new Server(ipLocal, ipMain, ipBackups);
	}
	server->run();

	delete server;
	return 0;
}
