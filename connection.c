/*
 * connection.c
 *
 *  Created on: Apr 11, 2015
 *  Author: evgenijavstein
 */

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>

#define BACKLOG 1 // how many pending connections queue will hold
#define PORT "2345"  // the port users will be connecting to

/**
 * Creates listening socket on port
 * @param port to bind to the socket
 * @return socket description of server socket
 */
int create_server_socket() {
	int socketfd;
	struct addrinfo hints, *servinfo;
	int yes = 1;
	int addrinfo_status;
	int listen_result;

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
		return -1;
	}

	freeaddrinfo(servinfo);
	listen_result = listen(socketfd, BACKLOG);
	if (listen_result == -1) {
		close(socketfd);
		fprintf(stderr, "Error in listen.\n");
		return -1;
	}
	return socketfd;
}

/**
 * User listening socket server_socket to accept a new client socket
 * @param listening socket
 * @return accepted client socket
 */
int accept_connection(int server_socket) {

	int client_socket; 	//socket for client
	struct sockaddr_in client_adress;	//client address
	int adress_len = sizeof(struct sockaddr_in);	//length of sockaddr_in structure

	client_socket = accept(server_socket, (struct sockaddr*) &client_adress, (socklen_t*) &adress_len);
	printf("Connection accepted : %s \n", inet_ntoa(client_adress.sin_addr));

	if (client_socket < 0) {
		perror("accept failed");
		return -1;
	}
	return client_socket;
}
