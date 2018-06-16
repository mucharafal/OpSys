#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <sys/epoll.h>
#define POSIX_C_SOURCE = 199309L
#include <time.h>
#include <pthread.h>
#include "datagram_connection_lib.h"
extern int errno ;

int fd_in, fd_out;
int af_unix;
char *unix_path;

void interpret_message(message *m, int size, char *client_name, int socket_type);
void send_to(int fd, int message_type, char *message_text, int socket_type, char *client_name);
void close_sockets(int a){
	close(fd_in);
	close(fd_out);
	if(af_unix)
		unlink(unix_path);
	exit(0);
}

void interpret_message(message *m, int size, char *client_name, int socket_type){
	compute_task *c;
	switch(m->message_type){
		case PRESENT:
			send_to(fd_out, PRESENT, "", SOCK_DGRAM, client_name);
			break;
		case CALCULATES_RESULT:
		printf("Received\n");
			c = (compute_task*)(m->message);
			int result;
			switch(c->sign){
				case '+':
					result = c->a+c->b;
					break;
				case '-':
					result = c->a-c->b;
					break;
				case '*':
					result = c->a*c->b;
					break;
				case '/':
					result = c->a/c->b;
					break;
			}
			char bufer[10];
			send_to(fd_out, CALCULATES_RESULT, Itoa(result, bufer, 10), SOCK_DGRAM, client_name);
			break;
	}
}

void send_to(int fd, int message_type, char *message_text, int socket_type, char *client_name){
	message *m;
	m = malloc(sizeof(message));
	
	m->socket_type = socket_type;
	m->message_type = message_type;
	strcpy(m->message, client_name);
	strcpy(&(m->message[strlen(m->message) + 1]), message_text);
	m->client_message_pointer = strlen(client_name) + 1;
	write(fd, m, sizeof(message));
}

int main(int args, char *argv[]){
	if(args < 3) return(1);

	char *client_name = argv[1];
	int socket_type;
	if(strcmp(argv[2], "AF_UNIX") == 0){
		af_unix = 1;
		socket_type = AF_UNIX;
		char bind_path[108] = {0};
		memcpy(bind_path, argv[1], strlen(argv[1]));
		unix_path = malloc(108);
		memcpy(unix_path, bind_path, strlen(bind_path) + 1);
		fd_in = create_unix_socket(SOCK_DGRAM, bind_path);
		fd_out = connect_with_unix_socket(argv[3]);
		send_to(fd_out, REGISTER, bind_path, AF_UNIX, client_name);
		//wait for response
	} else {
		af_unix = 0;
		socket_type = AF_INET;
		fd_in = create_inet_socket(SOCK_DGRAM, 0);
		struct sockaddr_in s;
		int size_of_s = sizeof(struct sockaddr_in);
		getsockname(fd_in, (struct sockaddr*)&s, &size_of_s);
		char port[10];
		Itoa(ntohs(s.sin_port), port, 10);
		fd_out = connect_with_inet_socket(atoi(argv[3]));
		send_to(fd_out, REGISTER, port, AF_INET, client_name);
		//wait for response
	}
	signal(SIGINT, close_sockets);

	int clients_pointer = 0;
	char bufer[1024];
	while(1){
		printf("Received\n");
		int read_r = read(fd_in, bufer, 1024);
		interpret_message((message*)bufer, read_r, client_name, SOCK_DGRAM);
	}

	pause();
}