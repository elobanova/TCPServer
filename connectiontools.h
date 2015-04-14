/*
 * connectiontools.h
 *
 *  Created on: Apr 14, 2015
 *      Author: ekaterina
 */

#ifndef CONNECTIONTOOLS_H_
#define CONNECTIONTOOLS_H_

#include <stdio.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

#define PORT "2345"  // the port users will be connecting to

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int setupSocketAndBind() {
	int socketfd;
	struct addrinfo hints, *servinfo;
	int yes = 1;
	int addrinfo_status;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((addrinfo_status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "Error occured while calling getaddrinfo: %s.\n", gai_strerror(addrinfo_status));
		return -1;
	}

	if ((socketfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) == -1) {
		fprintf(stderr, "Socket not created.\n");
		return -1;
	}

	if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		fprintf(stderr, "Socket option not set.\n");
		return -1;
	}

	if (bind(socketfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
		close(socketfd);
		fprintf(stderr, "Error in bind.\n");
	}

	freeaddrinfo(servinfo);
	return socketfd;
}

#endif /* CONNECTIONTOOLS_H_ */
