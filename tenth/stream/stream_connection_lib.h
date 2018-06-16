#ifndef __DATAGRAM_CONNECTION_LIB
#define __DATAGRAM_CONNECTION_LIB

#define NAME_LENGTH 108
#define CLIENTS_MAX 128
#define UNIX_MAX_SUNPATH 108

#define PRESENT 0
#define REGISTER 1
#define CALCULATES_RESULT 2

typedef struct {
	int socket_type;	//AF_INET or AF_UNIX
	int message_type;
	int client_message_pointer;
	char message[512];		//from 0 to client_name_pointer - 1	- client socket name
						//from client_name_pointer to client_message_pointer - 1 - client name, rest- client message
}message;

typedef struct{
	int a, b;
	char sign;
}compute_task;

int create_inet_socket(int socket_type, int port);
int create_unix_socket(int socket_type, char *bind_path);
int connect_with_inet_socket(int port);
int connect_with_unix_socket(char *path);
char* Itoa(long int value, char* str, int radix);

#endif