#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include <pthread.h>
#include "src/WrapperSocket.hpp"
#include "src/Packet.hpp"
#include "src/constants.hpp"

using namespace std;

void * functionalServer(void * socketPort);

int main(int argc, char *argv[])
{
	int * distributionPort = (int*) malloc(sizeof(int));
	*distributionPort = SERVER_PORT;
	Dropbox::WrapperSocket socket = Dropbox::WrapperSocket(SERVER_PORT);
	while (1) {
		(*distributionPort)++;
		socket.receive(TIMEOUT_OFF);
		/*pthread_t thread;
		pthread_create(&thread, NULL, &functionalServer, (void*) distributionPort);
		Dropbox::Packet packet = Dropbox::Packet(to_string(*distributionPort));

		cout << "sending new port";
		socket.send(packet);*/
	}
	return 0;
}

void * functionalServer(void * socketPort)
{
	int port = *((int *) socketPort); 
	cout << "creating socket at port " << to_string(port) << "\n"; 
	Dropbox::WrapperSocket socket(port);
	while(1){
		socket.receive(TIMEOUT_OFF);
	}
}