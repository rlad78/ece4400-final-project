/*
 * city_server.c
 * ECE 4400 Final Project
 * Experiment 2:
 * Program for 'server'(stoplight)
 * Based on source code and concepts from
 * "Beej's Guide to Network Programming Using Internet Sockets"
 * by Brian Hall
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>
#include <time.h>

#define PORT "3444"
// Allowed queued connections
#define BACKLOG 10
// Packet size
//#define MAXDATASIZE 512


struct DataPacket
{
    int type;
    char source[256];
    char dest[256];
    int distance;       //cars will be three units long
    int time;
    int status;
    int padding[16];
};

void sigchld_handler(int s)
{
	(void)s; // quiet unused variable warning

	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}


// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
{
	int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	int i;
	int number_of_packets = 0;
    //int isvalid;

	// Added
    struct DataPacket go_packet;
    struct DataPacket rec_packet;
    int maxdatasize = sizeof(go_packet);

    //char buf[maxdatasize];
    int numbytes;
    int time_to_go = 0;
    int currenttime;
    int oldtime;
    int newtime;

    // Current hostname
	char currenthost[256];
	currenthost[255] = '\0';
	gethostname(currenthost, 255);

    strcpy(go_packet.source, currenthost);


	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
				sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo); // all done with this structure

	if (p == NULL)  {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {  // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		inet_ntop(their_addr.ss_family,
			get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		time_to_go += 1;
		currenttime = time_to_go;

        // Child process handling socket
		if (!fork()) {
			close(sockfd);

			char fname[256];
			char host[256];

            //receive first packet from each vehicle to get source name and create a file for each one
			recv(new_fd, &rec_packet, maxdatasize, 0);
			strcpy(fname,rec_packet.source);

			FILE *fptr;
			fptr = fopen(fname, "wb");

            while(1)
            {
                oldtime = time(NULL);
                // Receive data
                // Note: you probably should never do this in the real world
                // This is why serialization libraries exist
                if ((numbytes = recv(new_fd, &rec_packet, maxdatasize, 0)) == -1) {
                    perror("recv");
                    exit(1);
                }

                rec_packet.time = currenttime;
                printf("source: '%s' Distance: '%i' Time: '%i'\n",rec_packet.source, rec_packet.distance, rec_packet.time);

                //condition if the vehicle is finished sending data
                if (rec_packet.type == 1)
                {
                    break;
                }

                sleep(1);
                newtime = time(NULL);
                currenttime = currenttime + (newtime - oldtime);

                fwrite(&rec_packet, 1, sizeof(rec_packet), fptr);
                number_of_packets++;
            }

            printf("%i data packets recorded\n", number_of_packets);
            fclose(fptr);

            //send packet for server hostname
            if (send(new_fd, &go_packet, maxdatasize, 0) == -1)
            perror("send");

            close(new_fd);
            printf("connection to %s closed\n", fname);
            exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	//parse through data to determine if collision happened

	return 0;
}

