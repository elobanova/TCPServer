/*
 * tcpserver.c
 *
 *  Created on: Apr 6, 2015
 *      Author: evgenijavstein
 */

// socket server example, handles multiple clients using threads
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <stdbool.h>

#include "tcpserver.h"
#include "connection.c"

#define OVER_EXICTED_HASH_LENGHT 10

size_t get_random_joke(char* fname, char* lname, uint8_t fname_len, uint8_t lname_len, char ** ppjoke);
char * randstring(size_t length);
int counter_concurrent_clients = 0; //global thread counter
pthread_mutex_t lock; //global lock

int main(int argc, char *argv[]) {
	pthread_t thread_id; //thread id for each created child thread
	int server_socket, client_socket; //socket descriptors: client, server

	server_socket = create_server_socket();
	if (server_socket == -1) {
		fprintf(stderr, "Error in socket setup.\n");
		return 1;
	}

	puts("Waiting for incoming connections...");

	//prepare mutex which will be locked by a thread incrementing/decrementing the counter_concurrent_clients
	if (pthread_mutex_init(&lock, NULL) != 0) {
		printf("\n mutex init failed\n");
		return 1;
	}

	//create thread for each incoming socket connection
	//accept only MAX_CONCURRENT_CLIENTS incoming sockets
	do {
		if (counter_concurrent_clients < MAX_CONCURRENT_CLIENTS) {

			client_socket = accept_connection(server_socket); //TODO: check if the return value is -1
			printf("concurrent clients:%d \n", counter_concurrent_clients);

			thread_args *arg = (thread_args*) malloc(sizeof(thread_args));
			arg->client_socket = client_socket;
			arg->thread_no = counter_concurrent_clients;

			//pass a socket to each handler of each thread
			if (pthread_create(&thread_id, NULL, connection_handler, (void*) arg) < 0) {
				perror("could not create thread");
				return 1;
			}
			//increment for each new accepted socket to client,
			//let child thread decrement before  exit
			increment_concurrent_clients();
			puts("Handler assigned");
		}

	} while (true);

	//TODO: what's with this? never reached
	//REACHED ONLY IF all clients disconnect
	pthread_mutex_destroy(&lock);
	close(server_socket);
	puts("server shut down");
	return 0;
}

/**
 * This will handle connection for each client off main thread
 * Tries to read  a packet of type JOKER_REQUEST_TYPE
 * Tries to read the name and send back a packet of type JOKER_RESPONSE_TYPE
 * @param arg  pointer to  threads_args
 * @see threada_args
 */
void *connection_handler(void * arg) {
	//Get the socket descriptor
	int sock = ((thread_args*) arg)->client_socket;
	int no = ((thread_args*) arg)->thread_no;
	//free struct, since values are copied
	free(arg);

	printf("thread no %d launched \n", no);

	int len_send;

	char buf[BUFSIZ];

	while ((len_send = recv(sock, buf, BUFSIZ, 0)) > 0) {

		printf("%s\n", buf);

		request_header * reqHeader = (request_header *) buf;
		char * payload = buf + sizeof(request_header);

		/** if a packet of type JOKER_REQUEST_TYPE arrives send a joke back**/
		if (reqHeader->type == JOKER_REQUEST_TYPE) {

			char *fname = malloc(reqHeader->len_first_name);
			char *lname = malloc(reqHeader->len_last_name);
			strncpy(fname, payload, reqHeader->len_first_name);
			strncpy(lname, payload + strlen(fname), reqHeader->len_last_name);

			//get random joke
			//char joke[strlen(JOKE1) + reqHeader->len_first_name
			//	+ reqHeader->len_last_name];
			char *joke_with_junk;
			size_t pure_joke_len = get_random_joke(fname, lname, reqHeader->len_first_name, reqHeader->len_last_name,
					&joke_with_junk);
			//sprintf(joke, JOKE1, fname, lname);
			/*create packet big enough for header+ joke*/
			char response[(sizeof(response_header) + strlen(joke_with_junk))];
			response_header resHeader;
			resHeader.type = JOKER_RESPONSE_TYPE;
			resHeader.len_joke = htonl(pure_joke_len); //make network byte order as client expects
			memcpy(response, (char *) &resHeader, sizeof(response_header));
			memcpy(response + sizeof(response_header), joke_with_junk, strlen(joke_with_junk));

			printf("%s \n", response);
			/*send packet*/
			//sleep(5);timeout test
			if ((len_send = send(sock, response, sizeof(response), 0)) < 0) {
				perror("receive");
				return 0;
			};

			printf("packet sent %s", response);

			/*else respond that the request is malformed*/
		} else {

			//if ((len_send = send(fd, "Malformed \n", 10, 0)) < 0) {
			if ((len_send = send(sock, buf, 10, 0)) < 0) {
				perror("send");
				return 0;
			};
		}

	}
	close(sock);

	if (len_send == 0) {
		puts("Client Disconnected");
	} else {
		perror("recv failed");
	}
	//this way new sockets can be accepted

	decrement_concurrent_clients();
	printf("thread no %d :termination", no);
	return 0;
}

/**
 * increments the counter_concurrent_clients global while
 * locking access for other threads
 */
void increment_concurrent_clients() {
	pthread_mutex_lock(&lock);
	counter_concurrent_clients++;
	pthread_mutex_unlock(&lock);
}

/**
 * decrements the counter_concurrent_clients global while
 * locking access for other threads
 */
void decrement_concurrent_clients() {
	pthread_mutex_lock(&lock);
	counter_concurrent_clients--;
	pthread_mutex_unlock(&lock);
}
/**
 * Chooses one from 10 jokes randomly.
 * Joke is allocated  ppjoke pointer will be pointed to it.
 * Random character are appended to the joke string. But function returns
 * the lenght of the pure joke
 * @param fname
 * @param lname
 * @param fname_len
 * @param lname_len
 * @param ppjoke pointer to a pointer pointing to NULL so far
 * @return pure joke lenght without random characters
 */
size_t get_random_joke(char* fname, char* lname, uint8_t fname_len, uint8_t lname_len, char ** ppjoke) {

	const char *jokes[10];
	jokes[0] = JOKE0;
	jokes[1] = JOKE1;
	jokes[2] = JOKE2;
	jokes[3] = JOKE3;
	jokes[4] = JOKE4;
	jokes[5] = JOKE5;
	jokes[6] = JOKE6;
	jokes[7] = JOKE7;
	jokes[8] = JOKE8;
	jokes[9] = JOKE9;

	int n = rand() % 9 + 1;
	srand(time(NULL));
	int b = rand() % 2;

	size_t overexcitement_hash_len;

	if (b == 0) {
		overexcitement_hash_len = 0;
	} else {
		overexcitement_hash_len = OVER_EXICTED_HASH_LENGHT;

	}
	//let second pointer, which is pointing to NULL point now to newly allocated memory
	*ppjoke = (char *) malloc((strlen(jokes[n]) + fname_len + lname_len + overexcitement_hash_len));

	//now inhabite newly allocated memory to which second pointer is poiting
	sprintf((char *) *ppjoke, jokes[n], fname, lname);

	//joke legnht without junk
	size_t joke_len = strlen(*ppjoke);

	//append junk randomly
	if (overexcitement_hash_len > 0) {
		char *hash = randstring(overexcitement_hash_len);
		strcat(*ppjoke, hash);
	}
	return joke_len;
}

char * randstring(size_t length) {

	static char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789,.-#'?!";
	char *randomString = NULL;

	if (length) {
		randomString = malloc(sizeof(char) * (length + 1));

		if (randomString) {
			int n;
			for (n = 0; n < length; n++) {
				int key = rand() % (int) (sizeof(charset) - 1);
				randomString[n] = charset[key];
			}

			randomString[length] = '\0';
		}
	}

	return randomString;
}

