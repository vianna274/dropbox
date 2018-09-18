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
#include "src/WrapperSocket.hpp"
#include "src/Packet.hpp"

#define PORT 4000

int main(int argc, char *argv[])
{		
	// char buffer[256];

	if (argc < 2) {
		fprintf(stderr, "usage %s hostname\n", argv[0]);
		exit(0);

	}

	Dropbox::Packet packet = Dropbox::Packet("Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.");
	Dropbox::WrapperSocket wSocket = Dropbox::WrapperSocket(argv[1], 4000);

	// printf("Enter the message: ");
	// bzero(buffer, 256);
	// fgets(buffer, 256, stdin);

	wSocket.send(packet);

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