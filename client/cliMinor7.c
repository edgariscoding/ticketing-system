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

	char* commands[] = {"BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "BUY", "RETURN ", "BUY"};
	char ticketDB[15][256];
	for(int i = 0; i <= 14; ++i) {
		strncpy(ticketDB[i], "0\0", 2);
	}

	size_t cmdCount = 0;
	while (cmdCount <= 14) {
		bzero(buffer, 256);

		if (strncmp(commands[cmdCount], "BUY", 3) == 0) {
			printf("[CLIENT] : BUY\n");

			n = write(sockfd, commands[cmdCount], strlen(commands[cmdCount]));
			if (n < 0) {
				error("ERROR writing to socket");
			}

			n = read(sockfd, buffer, 6);
			if (n < 0) {
				error("ERROR reading from socket");
			}

			if (strncmp(buffer, "FULL", 4) == 0) {
				printf("[SERVER] : Sold out\n");
				printf("[CLIENT] : Buy failed\n");
			}
			else {
				for(int i = 0; i <= 14; ++i) {
					if (strcmp(ticketDB[i], "0") == 0) {
						strncpy(ticketDB[i], buffer, 6);
						printf("[SERVER] : %s\n", ticketDB[i]);
						break;
					}
				}
			}
		}
		else if (strncmp(commands[cmdCount], "RETURN", 6) == 0) {
			printf("[CLIENT] : RETURN %s\n", ticketDB[0]);

			sprintf(buffer, "%s%s", commands[cmdCount], ticketDB[0]);
			n = write(sockfd, buffer, strlen(buffer));
			if (n < 0) {
				error("ERROR writing to socket");
			}

			bzero(buffer, 256);
			n = read(sockfd, buffer, 8);
			if (n < 0) {
				error("ERROR reading from socket");
			}

			if (strncmp(buffer, "INVALID", 8) == 0) {
				printf("[SERVER] : Ticket number is invalid\n");
			}
			else if (strncmp(buffer, "UNAVAIL", 8) == 0) {
				printf("[SERVER] : Ticket belongs to different client\n");
			}
			else {
				printf("[SERVER] : RETURN %s\n", ticketDB[0]);
				//ticketDB[0][0] = '0';
				//ticketDB[0][1] = '\0';
				strncpy(ticketDB[0], "0\0", 2);
				printf("[CLIENT] : TICKET %s returned\n", buffer);
			}
		}
		cmdCount++;
	}
	close(sockfd);

	printf("[CLIENT] : Ticket Database\n");
	for(int i = 0; i <= 14; ++i) {
		printf("%11s[ %2i ] - %s\n", "", i, ticketDB[i]);
	}

	return EXIT_SUCCESS;
}