/*
 * light_vehicle.c
 * ECE 4400 Final Project
 * Experiment 1: Fleet Control Stoplight
 * Client program for vehicle
 * Based on source code and concepts from
 * "Beej's Guide to Network Programming Using Internet Sockets"
 * by Brian Hall
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctype.h>

#define PORT "3444"
// Packet size
#define MAXDATASIZE 7

struct DataPacket
{
    int type;
    char source[256];
    char dest[256];
    int distance;
    int time;
    int status;
};

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;
	char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	int i;

	struct DataPacket packet;

	// Current hostname
	char currenthost[256];
	currenthost[255] = '\0';
	gethostname(currenthost, 255);

	if (argc != 3) {
	    fprintf(stderr,"usage: client hostname idnumber\n");
	    exit(1);
	}

	// Verify that input string is valid
    for (i = 0; i < 6; i++)
    {
        if (!isdigit(argv[2][i]))
        {
            fprintf(stderr, "idnumber is not valid\n");
            exit(1);
        }
    }
    printf("We are: %s\n", currenthost);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Resolve address for hostname "light"
	// ** change argv[1] to "Light"
	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// Connect to first match
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			perror("client: connect");
			close(sockfd);
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

	// Transmit request and wait for reply
    if (send(sockfd, "123456", 7, 0) == -1)
    perror("send");

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	buf[numbytes] = '\0';

	printf("client: string is '%s'\n",buf);

	close(sockfd);

	return 0;
}

