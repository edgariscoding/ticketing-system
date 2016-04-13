// =====================================================================
// Name:            Edgar Sanchez
// Course:          CSCE 3600
// Date:            April 11, 2016
// Title:           Minor Assignment 7 Server
// Version:         1.0
// Description:     This program will consist of a “main” ticket outlet
//                  (i.e., the server) that will provide services to
//                  “BUY” and “RETURN” tickets to two “local” ticket
//                  distributors (i.e., the clients).
// Format:          server
// =====================================================================

/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <stdbool.h>
#include <ctype.h>

// Function prototypes
struct Tickets {
	bool status;
	int number;
	size_t client;
};
void doStuff(int, struct Tickets ticket[20], size_t, int, char**);

void error(const char *msg) {
	perror(msg);
	exit(1);
}

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno;
	pid_t pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}
	// First call to socket() function
	// 1) Internet domain 2) Stream socket 3) protocol (TCP in this case)
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		error("ERROR opening socket");
	}

	// Initializes socket structure
	bzero((char *) &serv_addr, sizeof(serv_addr));
	portno = atoi(argv[1]);

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	// Binds the host address using bind() function
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		error("ERROR on binding");
	}

	// Listen for clients and wait for incoming connections
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	//signal(SIGCHLD,SIG_IGN);



	struct Tickets ticket[20];

	srand((unsigned int) time(NULL));

	for(int i = 0; i <= 19; ++i) {
		ticket[i].status = true;
		ticket[i].number = rand() % 90000 + 10000;
		ticket[i].client = 0;
	}

	// Client connects
	size_t clientNum = 1;
	while (clientNum <= 2) {
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		int sv[2]; // Pair of socket descriptors for AF_UNIX
		char* fromChild = "INIT"; // Holds data exchange between parent and child

		if (socketpair(AF_UNIX, SOCK_STREAM, 0, sv) == -1) {
			perror("socketpair");
			exit(1);
		}

		if (newsockfd < 0) {
			error("ERROR on accept");
		}
		pid = fork();

		if (pid < 0) {
			// Error
			error("ERROR on fork");
		}
		if (pid == 0) {
			// Child
			close(sockfd);
			close(sv[0]);
			printf("OG value: %s\n", fromChild);
			doStuff(newsockfd, ticket, clientNum, sv[1], &fromChild);
			exit(0);
		}
		else {
			// Parent
			close(newsockfd);
			close(sv[1]);
			read(sv[0], fromChild, 255);
			printf("parent: read '%s'\n", fromChild);

			clientNum++;
		}
	} /* end of while */

	wait(NULL);
	wait(NULL);
	close(sockfd);
	return EXIT_SUCCESS;
}

/******** doStuff() *********************
 There is a separate instance of this function
 for each connection.  It handles all communication
 once a connection has been established.
 *****************************************/
void doStuff(int sock, struct Tickets ticket[20], size_t clientNum, int socketPair, char **toParent)
{
	*toParent = malloc(8 * sizeof(char));
	strcpy(*toParent, "CHILD 1");
	write(socketPair, *toParent, strlen(*toParent));
	printf("child: sent CHILD %lu\n", clientNum);

	while (1) {
		ssize_t n;
		char buffer[256];
		bzero(buffer,256);
		n = read(sock,buffer,255);

		if (n == 0) {
			break;
		}

		if (n < 0) {
			error("ERROR reading from socket");
		}

		char* command = strtok(buffer, " \n");
		if (command == NULL) {
			fprintf(stderr, "Error splitting token\n");
		}
		char* returnNum = strtok(NULL, " \n");

		if (strncmp(command, "BUY", 3) == 0) {
			printf("[CLIENT %lu] : %s\n", clientNum, command);
			int i;
			for(i = 0; i <= 19; ++i) {
				if (ticket[i].status == true) {
					printf("[SERVER X] : Client %lu buy %i\n", clientNum, ticket[i].number);
					ticket[i].status = false;
					ticket[i].client = clientNum;

					char text[7];
					sprintf(text, "%d\n", ticket[i].number);
					n = write(sock, text, 7);

					if (n < 0) {
						error("ERROR writing to socket");
					}

					break;
				}
				else if (ticket[19].status == false && i == 19) {
					printf("[SERVER X] : Database full\n");
					n = write(sock, "Database full\n", 15);

					if (n < 0) {
						error("ERROR writing to socket");
					}

					break;
				}
			}

		}
		else if (strncmp(command, "RETURN", 6) == 0) {
			printf("[CLIENT %lu] : %s %s\n", clientNum, command, returnNum);

			int i;
			for(i = 0; i <= 19; ++i) {
				if (ticket[i].number == atoi(returnNum) && ticket[i].client == clientNum && ticket[i].status == false) {
					printf("[SERVER X] : Client %lu cancel %i\n", clientNum, ticket[i].number);
					ticket[i].status = true;
					ticket[i].client = 0;

					char text[15];
					sprintf(text, "%s %s\n", command, returnNum);
					n = write(sock, text, 15);

					if (n < 0) {
						error("ERROR writing to socket");
					}

					break;
				}
				else if (i == 19) {
					printf("[SERVER X] : Invalid ticket number\n");
					n = write(sock, "Invalid ticket number\n", 23);

					if (n < 0) {
						error("ERROR writing to socket");
					}
				}
			}

		}
		else {
			fprintf(stderr, "[CLIENT %lu] : Unknown command: %s\n", clientNum, command);
			n = write(sock, "Unknown command\n", 16);

			if (n < 0) {
				error("ERROR writing to socket");
			}
		}
	}
}