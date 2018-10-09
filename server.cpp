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
	Server *server = new Server();
	server->run();

	delete server;
	return 0;
}
