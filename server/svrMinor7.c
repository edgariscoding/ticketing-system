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
// Format:          ./server port
// =====================================================================

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <stdbool.h>

struct Tickets {
	bool status;
	int number;
	size_t client;
};

// Function prototypes
void processRequest(int, struct Tickets *ticket, size_t);
int error(const char *);

int main(int argc, char *argv[]) {
	// Checks if a port argument was provided
	if (argc < 2) {
		fprintf(stderr,"ERROR, no port provided\n");
		return EXIT_FAILURE;
	}

	// Declare variables
	int sockfd, newsockfd, portno;
	pid_t pid;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	// Opens INET TCP socket
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

	// Listens for clients and waits for incoming connections
	listen(sockfd,5);
	clilen = sizeof(cli_addr);

	// Creates base ticket struct then creates a shared memory ticket struct pointing to base
	struct Tickets ticketBase[20];
	struct Tickets *ticket = mmap(ticketBase, sizeof(struct Tickets), PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);;

	// Generates random ticket numbers and initializes ticket properties
	srand((unsigned int) time(NULL));
	for(int i = 0; i <= 19; ++i) {
		ticket[i].status = true;
		ticket[i].number = rand() % 90000 + 10000;
		ticket[i].client = 0;
	}

	// Client connection loop (allows two connections)
	size_t clientNum = 1;
	while (clientNum <= 2) {
		// Waits until client connects then stores new file descriptor in newsockfd
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			error("ERROR on accept");
		}

		// Forks and processes client requests in child
		pid = fork();
		if (pid < 0) {
			// Error
			error("ERROR on fork");
		}
		else if (pid == 0) {
			// Child Process
			close(sockfd);
			processRequest(newsockfd, ticket, clientNum);
			return EXIT_SUCCESS;
		}
		else {
			// Parent Process
			close(newsockfd);
			clientNum++;
		}
	}

	// Waits for forked processes to terminate and gracefully exits
	wait(NULL);
	wait(NULL);
	close(sockfd);
	munmap(ticket, sizeof(struct Tickets));
	return EXIT_SUCCESS;
}

// Processes ticketing requests from clients
void processRequest(int sock, struct Tickets *ticket, size_t clientNum)
{
	while (1) {
		ssize_t n;
		char buffer[256];
		bzero(buffer,256);

		// Reads from client and stores in buffer
		n = read(sock,buffer,255);
		if (n == 0) {
			break;
		}
		else if (n < 0) {
			error("ERROR reading from socket");
		}

		// Splits client input into command and ticket number if provided
		char* command = strtok(buffer, " ");
		if (command == NULL) {
			error("ERROR splitting command");
		}
		char* returnNum = strtok(NULL, " ");

		// Processes commands and modifies tickets as requested
		if (strncmp(command, "BUY", 3) == 0) {
			printf("[CLIENT %lu] : %s", clientNum, command);
			for(int i = 0; i <= 19; ++i) {
				if (ticket[i].status == true) {
					printf("[SERVER X] : Client %lu bought %i\n", clientNum, ticket[i].number);
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
			printf("[CLIENT %lu] : %s %s", clientNum, command, returnNum);
			for(int i = 0; i <= 19; ++i) {
				if (ticket[i].number == atoi(returnNum) && ticket[i].client == clientNum && ticket[i].status == false) {
					printf("[SERVER X] : Client %lu returned %i\n", clientNum, ticket[i].number);
					ticket[i].status = true;
					ticket[i].client = 0;

					char text[15];
					sprintf(text, "%s %s", command, returnNum);
					n = write(sock, text, 15);
					if (n < 0) {
						error("ERROR writing to socket");
					}
					break;
				}
				else if (ticket[i].number == atoi(returnNum) && ticket[i].client != clientNum && ticket[i].status == false) {
					printf("[SERVER X] : Client %lu does not own %i\n", clientNum, ticket[i].number);
					n = write(sock, "Ticket number belongs to different client\n", 23);
					if (n < 0) {
						error("ERROR writing to socket");
					}
				}
				else if (i == 19) {
					printf("[SERVER X] : Invalid ticket number %s\n", returnNum);
					n = write(sock, "Invalid ticket number\n", 23);
					if (n < 0) {
						error("ERROR writing to socket");
					}
				}
			}
		}
		else {
			fprintf(stderr, "[CLIENT %lu] : Unknown command: %s", clientNum, command);
			n = write(sock, "Unknown command\n", 16);
			if (n < 0) {
				error("ERROR writing to socket");
			}
		}
	}
}

int error(const char *msg) {
	perror(msg);
	return EXIT_FAILURE;
}