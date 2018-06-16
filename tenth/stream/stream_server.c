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

#include "stream_connection_lib.h"

#define MAX_EVENTS 1
#define NAME_LENGTH 108
#define CLIENTS_MAX 128
#define UNIX_MAX_SUNPATH 108
extern int errno ;

int fd, fd1;
char *unix_path;

#define PRESENT 0
#define REGISTER 1
#define CALCULATES_RESULT 2

#define REPORT 0

typedef struct{
	int socket_type;
	char client_name[NAME_LENGTH];
	int fd;
	int last_used;
}connection;

int clients_number;
connection clients_list[CLIENTS_MAX];

int create_inet_socket(int socket_type, int port);
int create_unix_socket(int socket_type, char *bind_path);
int interpret_message(message *m, int size);
void add_fd_to_epoll(int fd, int epollfd);
void read_message(int fd);
void add_client(int fd, int epollfd);

void close_sockets(int a){
	shutdown(fd, SHUT_RDWR);
	shutdown(fd1, SHUT_RDWR);
	close(fd);
	close(fd1);
	unlink(unix_path);
	exit(0);
}

void *receiver(void *epollfdp){
	int epollfd = *((int*)epollfdp);
	struct epoll_event events[MAX_EVENTS];
	while(1){
		int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
		if (nfds == -1) {
			perror("epoll_wait");
			exit(EXIT_FAILURE);
		}
		int fd_read = events[0].data.fd;
		if(fd_read == fd || fd_read == fd1){
			add_client(fd_read, epollfd);
		} else {
			read_message(fd_read);
		}
	}
}

int register_presence(message *m){
	char *client_name = m->message;
	int i = 0;
	int found = 0;
	for(;i < CLIENTS_MAX;i++){
		if(strcmp(client_name, clients_list[i].client_name)==0){
			found = 1;
			break;
		}
	}
	if(found){
		clients_list[i].last_used = 1;
	} else {
		printf("client not found...\n");
	}
	return i;
}

message *receive_message(int fd){
	int message_size;
	read(fd, &message_size, sizeof(int));
	message *m = calloc(sizeof(message), 1);
	read(fd, m, message_size);
	return m;
}

void send_message(int fd, message *m){
	int message_size = sizeof(message);
	write(fd, &message_size, sizeof(int));
	write(fd, m, sizeof(message));
}

void add_client(int fd, int epollfd){
	if(CLIENTS_MAX <= clients_number){
		//error, cannot register
		close(fd);
		return;
	}

	int size = sizeof(struct sockaddr_un);
	struct sockaddr *addr = malloc(size);
	int client_fd = accept(fd, addr, &size);
	if(client_fd == -1){
		perror("accept: client_fd");
		exit(EXIT_FAILURE);
	}
	struct sockaddr_in *s = (struct sockaddr_in*)addr;

	printf("Port: %i\n", ntohs(s->sin_port));

	//receive message
	message *m = receive_message(client_fd);

	char *client_name = m->message;

	int i = 0;
	int found = 0;
	for(;i < CLIENTS_MAX;i++){
		if(clients_list[i].fd == -1){
			found = 1;
			break;
		}
	}
	if(found){
		connection *slot = &clients_list[i];

		slot->last_used = 1;
		strcpy(slot->client_name, client_name);
		slot->socket_type = m->socket_type;
		slot->fd = client_fd;

		clients_number++;	//if it should be thread safe, this statement should be on begin 
	}
	add_fd_to_epoll(client_fd, epollfd);
	free(m);
}

void read_message(int fd){
	message *m = receive_message(fd);
	interpret_message(m, sizeof(message));
	free(m);
}

int interpret_message(message *m, int size){
	int client_number;
	switch(m->message_type){
		case PRESENT:
			register_presence(m);
			break;
		case CALCULATES_RESULT:
			client_number = register_presence(m);
			if(client_number == CLIENTS_MAX){
				printf("Unregistered client send: %s", &(m->message[m->client_message_pointer]));
			}
			printf("From %i, received: %s\n", clients_list[client_number].client_name, &(m->message[m->client_message_pointer]));
			break;
	}
}

void add_fd_to_epoll(int fd, int epollfd){
	struct epoll_event ev;

	ev.events = EPOLLIN | EPOLLET;
	ev.data.fd = fd;

	printf("fd: %i\n", fd);
	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1) {
		perror("epoll_ctl1: listen_sock");
		exit(EXIT_FAILURE);
	}
}

void send_to(connection *client, int message_type, char *message_text){
	int fd = client->fd;
	message *m = malloc(sizeof(message));
	m->message_type = message_type;
	memcpy(m->message, message_text, 512);
	send_message(fd, m);
}

void *conductor(void *p){
	for(int i = 0;;i = (i + 1) % CLIENTS_MAX){
		connection *client_slot = &(clients_list[i]);
		if(client_slot->fd != -1){
			if(client_slot->last_used == 1){
				client_slot->last_used = 0;
				send_to(client_slot, PRESENT, "");
			} else {
				printf("Removed %s, index: %i, fd: %i\n", client_slot->client_name, i, client_slot->fd);
				shutdown(client_slot->fd, SHUT_RDWR);
				close(client_slot->fd);
				client_slot->fd = -1;
				clients_number--;
			}
		}
		struct timespec sleep_time;
		sleep_time.tv_sec = 0;
		sleep_time.tv_nsec = 1000000;
		nanosleep(&sleep_time, NULL);
	}
}

int main(int args, char *argv[]){
	if(args < 3) {
		printf("Za malo arg\n");
		return 1;
	}
	int port = atoi(argv[1]);
	fd1 = create_inet_socket(SOCK_STREAM, port);
	fd = create_unix_socket(SOCK_STREAM, argv[2]);
	unix_path = argv[2];
	
	signal(SIGINT, close_sockets);
	int epollfd = epoll_create1(0);
	if (epollfd == -1) {
		perror("epoll_create1");
		exit(EXIT_FAILURE);
	}
	
	add_fd_to_epoll(fd, epollfd);
	add_fd_to_epoll(fd1, epollfd);

	for(int i = 0;i < CLIENTS_MAX;i++){
		clients_list[i].fd = -1;
		clients_list[i].last_used = 0;
		clients_list[i].socket_type = 0;
	}

	clients_number = 0;

	pthread_t receiver_handler;
	pthread_create(&receiver_handler, NULL, receiver, (void*)(&epollfd));

	pthread_t conductor_handler;
	pthread_create(&conductor_handler, NULL, conductor, NULL);

	int clients_pointer = 0;
	while(1){
		int a, b;
		char sign;
		scanf("%i%c%i", &a, &sign, &b);

		compute_task t;
		t.a = a;
		t.b = b;
		t.sign = sign;
		char buf[sizeof(compute_task) + 1];
		memcpy(buf, &t, sizeof(compute_task));
		buf[sizeof(compute_task)] = 0;
		
		int found = 0;
		for(int i = 0;i < CLIENTS_MAX;i++){
			clients_pointer = (clients_pointer + 1) % CLIENTS_MAX;
			if(clients_list[clients_pointer].fd != -1){
				found = 1;
				break;
			}
		}
		if(found){
			send_to(&(clients_list[clients_pointer]), CALCULATES_RESULT, buf);
		} else {
			printf("No registered client...\n");
		}
	}

	pause();
}