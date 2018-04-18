/*
 * light_stoplight.c
 * ECE 4400 Final Project
 * Experiment 1: Fleet Control Stoplight
 * Server program for 'stoplight'
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

#define PORT "3444"
// Allowed queued connections
#define BACKLOG 10
// Packet size
#define MAXDATASIZE 512


const char *id_array[5] = {"130523", "123456", "163863", "832334", "382551"};
const char *string_array[5] = {"athing", "bthing", "cthing", "dthing", "ething"};

struct DataPacket
{
    int type;
    char source[256];
    char dest[256];
    int distance;
    int time;
    int status;
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
    int isvalid;

	// Added
    char buf[MAXDATASIZE];
    int numbytes;

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



		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

            // Receive data
            if ((numbytes = recv(new_fd, buf, MAXDATASIZE-1, 0)) == -1) {
                perror("recv");
                exit(1);
            }
            buf[numbytes] = '\0';
            printf("server: received ID number '%s'\n",buf);

            // Check if string is valid
            for (i = 0; i < 6; i++)
            {
                if (!isdigit(buf[i]))
                {
                    isvalid = 0;
                    break;
                }
            }

            // If not 6 char number string, send "notnum"
            if (!isvalid)
            {
                if (send(new_fd, "notnum", 7, 0) == -1)
                perror("send");
            }
            // Look up ID number if valid
            else
            {

                for (i = 0; i < 5; i++)
                {
                    // Send back corresponding string if ID matched
                    if (strcmp(buf, id_array[i]) == 0)
                    {
                        strcpy(buf, string_array[i]);
                        if (send(new_fd, buf, 7, 0) == -1)
                        perror("send");
                        break;
                    }

                    // Send "wrong" if not found
                    if (i == 4)
                    {
                        if (send(new_fd, "wrong", 7, 0) == -1)
                        perror("send");
                    }
                }
            }

			close(new_fd);
			exit(0);
		}
		close(new_fd);  // parent doesn't need this
	}

	return 0;
}

