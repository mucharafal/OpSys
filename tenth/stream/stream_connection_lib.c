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

int create_inet_socket(int socket_type, int port) {

	int fd = socket(AF_INET, socket_type, 0);

	if(fd == -1){
		fprintf(stderr, "Error binding socket: %s\n", strerror( errno ));
		exit(1);
	}

	struct sockaddr_in *addr_in;
	addr_in = malloc(sizeof(struct sockaddr_in)); 
	addr_in->sin_family = AF_INET;
	addr_in->sin_port = htons((u_int16_t)port);
	addr_in->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	for(int i = 0; i < 8;i++)
		addr_in->sin_zero[i] = 0;

	if(bind(fd, (struct sockaddr*)addr_in, sizeof(struct sockaddr_in)) == -1){
		fprintf(stderr, "Error binding socket: %s\n", strerror( errno ));
		exit(1);
	}
	port = ntohs(addr_in->sin_port);
	printf("Creating Port: %i\n", port);

	if(listen(fd, 1) == -1){
		fprintf(stderr, "Error listen socket: %s\n", strerror( errno ));
		exit(1);
	}
	free(addr_in);
	return fd;
}

int create_unix_socket(int socket_type, char *bind_path){
	unlink(bind_path);

	int fd = socket(AF_UNIX, socket_type, 0);
	if(fd == -1){
		fprintf(stderr, "Error binding socket: %s\n", strerror( errno ));
		exit(1);
	}

	struct sockaddr_un *addr;
	addr = malloc(sizeof(struct sockaddr_un)); 
	addr->sun_family = AF_UNIX;
	strcpy(addr->sun_path, bind_path);

	if(bind(fd, (struct sockaddr*)addr, sizeof(struct sockaddr_un)) == -1){
		fprintf(stderr, "Error binding socket, no tak: %s\n", strerror( errno ));
		exit(1);
	}

	if(listen(fd, 1) == -1){
		fprintf(stderr, "Error listen socket: %s\n", strerror( errno ));
		exit(1);
	}
	free(addr);
	return fd;
}


int connect_with_inet_socket(int port){
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1){
		printf("Łerror!\n");
		exit(1);
	}

	struct sockaddr_in *addr;

	addr = malloc(sizeof(struct sockaddr_in)); 
	addr->sin_family = AF_INET;
	addr->sin_port = htons((u_int16_t)port);
	addr->sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	for(int i = 0; i < 8;i++)
		addr->sin_zero[i] = 0;

	printf("Connecting Port: %i\n", port);

	if(connect(fd, (struct sockaddr*)addr, sizeof(struct sockaddr_in)) != 0){
		fprintf(stderr, "Error connecting socket, : %s\n", strerror( errno ));
		close(fd);
		return -1;
	}

	printf("Port in connect function: %i\n", ntohs(addr->sin_port));

	free(addr);
	return fd;
}

int connect_with_unix_socket(char *path){
	int fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if(fd == -1){
		printf("Łerror!\n");
		exit(1);
	}

	struct sockaddr_un *addr;
	addr = malloc(sizeof(struct sockaddr_un)); 
	addr->sun_family = AF_UNIX;
	strcpy(addr->sun_path, path);
	printf("%s\n", path);
	if(connect(fd, (struct sockaddr*)addr, sizeof(struct sockaddr_un)) != 0){
		fprintf(stderr, "Error connecting socket, : %s\n", strerror( errno ));
	}
	free(addr);
	return fd;
}

//from: https://www.daniweb.com/programming/software-development/threads/148080/itoa-function-or-similar-in-linux
char* Itoa(long int value, char* str, int radix) {
    static char dig[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    long int n = 0, neg = 0;
    unsigned int v;
    char* p, *q;
    char c;
    if (radix == 10 && value < 0) {
        value = -value;
        neg = 1;
    }
    v = value;
    do {
        str[n++] = dig[v%radix];
        v /= radix;
    } while (v);
    if (neg)
        str[n++] = '-';
    str[n] = '\0';
    for (p = str, q = p + n - 1; p < q; ++p, --q) {
        c = *p, *p = *q, *q = c;
	}
    return str;
}