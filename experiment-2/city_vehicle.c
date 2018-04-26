/*
 * city_vehicle.c
 * ECE 4400 Final Project
 * Experiment 2:
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
#include <time.h>

#define PORT "3444"
// Packet size
//define MAXDATASIZE 7

struct DataPacket
{
    int type;
    char source[256];
    char dest[256];
    int distance;       //cars will be three units long
    int lane;           //only two lanes
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
    struct DataPacket our_packet;
    struct DataPacket server_packet;
	int maxdatasize = sizeof(our_packet);




	int sockfd, numbytes;
	//char buf[maxdatasize];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	//int i;

	// Current hostname
	char currenthost[256];
	currenthost[255] = '\0';
	gethostname(currenthost, 255);

	strcpy(our_packet.source, currenthost);

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

    printf("We are: %s\n", currenthost);

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	// Resolve address for hostname "light"
	// ** change argv[1] to "city-server"
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

	//set up initial position data
	int init_pos1 = 5;
    int init_lane1 = 1;

    int init_pos2 = 10;
    int init_lane2 = 1;

    int init_pos3 = 25;
    int init_lane3 = 1;

    int init_pos4 = 15;
    int init_lane4 = 2;

    if (strcmp(currenthost,"apollo07") == 0) //vehicle 1
    {
        our_packet.type = 0;
        our_packet.distance = init_pos1;
        our_packet.lane = init_lane1;
    }
    if (strcmp(currenthost,"vehicle-2") == 0) //vehicle 2
    {
        our_packet.type = 0;
        our_packet.distance = init_pos2;
        our_packet.lane = init_lane2;
    }
    if (strcmp(currenthost,"vehicle-3") == 0) //vehicle 3
    {
        our_packet.type = 0;
        our_packet.distance = init_pos3;
        our_packet.lane = init_lane3;
    }
    if (strcmp(currenthost, "apollo08") == 0) //vehicle 4
    {
        our_packet.type = 0;
        our_packet.distance = init_pos4;
        our_packet.lane = init_lane4;
    }

	// Transmit request and wait for reply
    if (send(sockfd, &our_packet, maxdatasize, 0) == -1)
    perror("send");

    while(our_packet.distance < 50)
    {
        //continue to send update packets for position
        strcpy(our_packet.source, currenthost);

        if (send(sockfd, &our_packet, maxdatasize, 0) == -1)
        perror("send");

        printf("Current Distance: %d\n", our_packet.distance);

        our_packet.distance += 5;
        sleep(1);
    }

    our_packet.type = 1;

    if (send(sockfd, &our_packet, maxdatasize, 0) == -1)
        perror("send");

	if ((numbytes = recv(sockfd, &server_packet, maxdatasize, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

	printf("The server's hostname is: '%s'\n",server_packet.source);

	close(sockfd);

	return 0;
}

