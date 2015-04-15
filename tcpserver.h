/*
 * tcpserver.h
 *
 *  Created on: Apr 11, 2015
 *      Author: evgenijavstein
 */

#ifndef TCPSERVER_H_
#define TCPSERVER_H_
#define MAX_CONCURRENT_CLIENTS 2
#define JOKER_REQUEST_TYPE 1
#define JOKER_RESPONSE_TYPE 2

#define JOKE0 "%s %s compiles his code in his head and throws warnings."
#define JOKE1 "%s %s starts his terminal right after waking up."
#define JOKE2 "Ask  %s %s to review 10 lines of code, he'll find 10 issues. Ask him to do 500 lines and he'll say it looks good"
#define JOKE3 "%s %s 's wife calls him and tells him, 'While you're out, buy some milk'.He never returns home."
#define JOKE4 "%s %s's girlfriend: 'Are you going to sit and type in front of that thing all day or are you going out with me?\n'Programmer: 'Yes.'"
#define JOKE5 "Stackoveflow.com? No, %s %s knows all command!!!! "
#define JOKE6 "%s %s does not have a girlfriend. He thinks this is too expensive. You should buy a new mother board instead."
#define JOKE7 "%s %s's best friend is Linux"
#define JOKE8 "%s %s does not need an IDE, he debugs via command line only."
#define JOKE9 "%s %s thinks IDEs are for girls"

typedef struct {
	uint8_t type;
	uint32_t len_joke;
}__attribute__ ((__packed__)) response_header;

typedef struct {
	uint8_t type;
	uint8_t len_first_name;
	uint8_t len_last_name;
}__attribute__ ((__packed__)) request_header;

typedef struct {
	int thread_no;
	int client_socket;
} thread_args;

int accept_connection(int server_socket);
int create_server_socket();
void *connection_handler(void *);
void increment_concurrent_clients();
void decrement_concurrent_clients();

#endif /* TCPSERVER_H_ */
