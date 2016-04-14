// =====================================================================
// Name:            Edgar Sanchez
// Course:          CSCE 3600
// Date:            April 11, 2016
// Title:           Minor Assignment 7 Client
// Version:         1.0
// Description:     This program will consist of a “main” ticket outlet
//                  (i.e., the server) that will provide services to
//                  “BUY” and “RETURN” tickets to two “local” ticket
//                  distributors (i.e., the clients).
// Format:          client
// =====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[])
{
	int sockfd, portno;
	ssize_t n;
	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[256];
	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		error("ERROR opening socket");
	server = gethostbyname(argv[1]);
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, (size_t) server->h_length);
	serv_addr.sin_port = htons(portno);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR connecting");
	}

	char* commands[] = {"BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "BUY\n", "RETURN ", "BUY\n"};
	char* ticketDB[15];

	size_t cmdCount = 0;
	while (cmdCount <= 14) {
		//printf("Command: ");
		bzero(buffer, 256);
		//fgets(buffer, 255, commands[0]);

		if (strncmp(commands[cmdCount], "BUY", 3) == 0) {
			n = write(sockfd, commands[cmdCount], strlen(commands[cmdCount]));
			if (n < 0) {
				error("ERROR writing to socket");
			}

			bzero(buffer, 256);
			n = read(sockfd, buffer, 255);
			if (n < 0) {
				error("ERROR reading from socket");
			}
			//memcpy(ticketDB[cmdCount], buffer, sizeof(buffer));

			printf("[SERVER] : %s", buffer);
		}
		else if (strncmp(commands[cmdCount], "RETURN", 6) == 0) {

			strncat(buffer, ticketDB[1], 5); // TODO

			n = write(sockfd, buffer, strlen(buffer));
			if (n < 0) {
				error("ERROR writing to socket");
			}

			bzero(buffer, 256);
			n = read(sockfd, buffer, 255);
			if (n < 0) {
				error("ERROR reading from socket");
			}



			printf("[SERVER] : %s", buffer);
		}
		cmdCount++;
	}
	close(sockfd);

	/*
	printf("                 Status          Number\n");
	for(int i = 0; i <= 19; ++i) {
		printf("Ticket [ %2i ]:   ", (i + 1));
		printf("%s", ticket[i].status ? "Available   -   " : "Unavailable -   ");
		printf("%i\n", ticket[i].number);
	}
	*/

	return EXIT_SUCCESS;
}