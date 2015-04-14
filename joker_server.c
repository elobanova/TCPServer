/*
 * joker_server.c
 *
 *  Created on: Apr 14, 2015
 *      Author: ekaterina
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* memset() */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>

#include "connectiontools.h"
#include "dataexchangetypes.h"

#define BACKLOG 10     // how many pending connections queue will hold
#define NAME_MAX_LENGTH 20
#define MAX_JOKE_LENGTH 200

void *handle(void *socket_desc) {
	//info from client
	int recv_bytes_for_header;
	int recv_bytes_for_first_name;
	int recv_bytes_for_last_name;
	struct request_header client_header;
	uint8_t len_of_first_name = 0;
	uint8_t len_of_last_name = 0;
	char *first_name_buffer;
	char *last_name_buffer;

	//info from server
	int response_struct_size;
	response_header *response_msg;
	char *joke_string;

	//Get the socket descriptor
	int socketfd = *(int*) socket_desc;

	recv_bytes_for_header = recv(socketfd, (char *) &client_header, sizeof(request_header), 0);

	int request_header_size = sizeof(request_header);
	if (recv_bytes_for_header < request_header_size || client_header.type != JOKER_REQUEST_TYPE) {
		close(socketfd);
		fprintf(stderr, "A client did not respond with the proper header.\n");
	}

	len_of_first_name = client_header.len_first_name;
	len_of_last_name = client_header.len_last_name;

	first_name_buffer = (char *) malloc(len_of_first_name);
	last_name_buffer = (char *) malloc(len_of_last_name);
	recv_bytes_for_first_name = recv(socketfd, first_name_buffer, len_of_first_name, 0);
	first_name_buffer[len_of_first_name] = '\0';
	recv_bytes_for_last_name = recv(socketfd, last_name_buffer, len_of_last_name, 0);
	last_name_buffer[len_of_last_name] = '\0';

	if (recv_bytes_for_first_name < len_of_first_name || recv_bytes_for_last_name < len_of_last_name) {
		close(socketfd);
		fprintf(stderr, "The first and last names were not fully received from the client.\n");
	}

	printf("Client data received correctly. \n");
	printf("%s \n", first_name_buffer);
	printf("%s \n", last_name_buffer);

	//send a joke to the client
	char joke[] = "Always and forever %s %s googles with regexps.";
	joke_string = (char *) malloc(strlen(joke) + len_of_first_name + len_of_last_name);
	sprintf(joke_string, joke, first_name_buffer, last_name_buffer);

	free(first_name_buffer);
	free(last_name_buffer);
	printf("a joke string %s\n", joke_string);
	uint32_t joke_length = strlen(joke_string);

	response_struct_size = sizeof(response_header);
	int buffer_size = response_struct_size + joke_length;
	char *buffer = (char *) malloc(buffer_size);
	response_msg = (response_header *) buffer;
	response_msg->type = JOKER_RESPONSE_TYPE;
	response_msg->joke_length = htonl(joke_length);

	char * payload = buffer + response_struct_size;
	strncpy(payload, joke_string, joke_length);

	int bytes_sent = send(socketfd, buffer, buffer_size, 0);
	free(buffer);
	free(joke_string);
	if (bytes_sent == -1) {
		printf("Error while sending joke");
	}

	return 0;
}

int main(int argc, char *argv[]) {
	int socketfd;
	pthread_t thread;

	socketfd = setupSocketAndBind();

	if (socketfd == -1) {
		return 1;
	}

	if (listen(socketfd, BACKLOG) == -1) {
		perror("listen");
		return 1;
	}

	printf("server: waiting for connections...\n");

	while (1) {
		size_t size = sizeof(struct sockaddr_in);
		struct sockaddr_in their_addr;
		int newsock = accept(socketfd, (struct sockaddr*) &their_addr, &size);
		if (newsock == -1) {
			perror("accept");
		} else {
			printf("Got a connection from %s on port %d\n", inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));
			if (pthread_create(&thread, NULL, handle, &newsock) != 0) {
				fprintf(stderr, "Failed to create thread\n");
			}
		}
	}

	close(socketfd);
	return 0;
}
