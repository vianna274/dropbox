#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <iostream>
#include "src/WrapperSocket.hpp"
#include "src/Packet.hpp"
#include "src/constants.hpp"

using namespace std;

int main(int argc, char *argv[])
{
	Dropbox::WrapperSocket socket = Dropbox::WrapperSocket("none", SERVER_PORT);
	while (1) {
		socket.receive();
	}
	return 0;
}
