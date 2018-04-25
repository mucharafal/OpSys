#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <linux/msg.h>
#include <linux/ipc.h>
#include <string.h>
#include <mqueue.h>
#include "header.h"
#include <errno.h>

int queues[MAX_CLIENT_NUMBER];
int clients;
mqd_t queue;
void quit(int a);

void addClient(commandLine *receivedMessage) {
	//opening candidate queue
	char *queuePath = calloc(16, 1);
	strcpy(queuePath, "/");
	strcat(queuePath, receivedMessage->array[1]);
	int candidateQueue = openQueue(queuePath, O_WRONLY);
	if(candidateQueue == -1) {
		printf("Cannot open client queue\n");
	} else {
		//register or reject
		char *message = calloc(MSGBUF_SIZE, 1);
		if(clients < MAX_CLIENT_NUMBER){
			Itoa(clients, message, 10);
			queues[clients] = candidateQueue;
			clients++;
		} else {
			strcpy(message, "NO");
		}
		if(mq_send(candidateQueue, message, strlen(message), 0) == -1)
			printf("Error during sending...\n");
	}
}

void mirror(commandLine *line, char *message){
	//who send
	int clientID = atoi(line->array[1]);
	//what
	char *buffer = strstr(message, line->array[2]);
	//mirror
	int textLen = strlen(buffer);
	char c;
	for(int i = 0, j = textLen - 1; i < j; i++, j--){	//miejsce błędogenne
		c = buffer[i];
		buffer[i] = buffer[j];
		buffer[j] = c;
	}
	//send
	mq_send(queues[clientID], buffer, strlen(buffer), 0);
}

void sendTime(commandLine *line){
	//who send
	int clientID = atoi(line->array[1]);
	//get date
	char *buffer = calloc(MSGBUF_SIZE, 1);
	fread(buffer, 1, MSGBUF_SIZE, popen("date", "r"));
	//send
	mq_send(queues[clientID], buffer, MSGBUF_SIZE, 0);
	free(buffer);
}

void calc(commandLine *line) {
	int clientID = atoi(line->array[0]);

	int a = atoi(line->array[2]);
	int b = atoi(line->array[4]);
	char sign = line->array[3][0];
	int result;
	switch(sign) {
		case '+':
		result = a + b;
		break;
		case '-':
		result = a - b;
		break;
		case '*':
		result = a * b;
		break;
		case '/':
		result = a / b;
		break;
	}

	char *buffer = calloc(MSGBUF_SIZE, 1);

	Itoa(result, buffer, 10);
	mq_send(queues[clientID], buffer, MSGBUF_SIZE, 0);
	free(buffer);
}

void receiver(){
	int receivedChars;
	char *message = malloc(MSGBUF_SIZE + 1);
	struct timespec waitTime;
	waitTime.tv_sec = time(NULL) + 10;
	waitTime.tv_nsec = 0;
	
	while((receivedChars = mq_timedreceive(queue, message, MSGBUF_SIZE + 1, NULL, &waitTime)) != -1){
		if(receivedChars == -1) {
			printf("Cannot read from queue\n");
			exit(1);
		} else {
			commandLine *processedMessage = processLineToCommandLine(message);
			if(processedMessage->arraySize != 0 && strcmp(processedMessage->array[0], "ADD") == 0){
				addClient(processedMessage);
			}

			if(0 == strcmp(processedMessage->array[1], "END")) {
				waitTime.tv_nsec = 0;
				waitTime.tv_sec = 0;
			} else {
				waitTime.tv_sec = time(NULL) + 10;
			}

			if(0 == strcmp(processedMessage->array[1], "MIRROR")) {
				mirror(processedMessage, message);
			}

			if(0 == strcmp(processedMessage->array[1], "CALC")) {
				calc(processedMessage);
			}

			if(0 == strcmp(processedMessage->array[1], "DATE")) {
				sendTime(processedMessage);
			}
		}

	}
	quit(0);
}

void quit(int a){
	char *path = "/SERVER";
	removeQueue(path);
	exit(0);
}

int main(int args, char *argv[]) {
	
	clients = 0;
	signal(SIGINT, &quit);
	char *path = "/SERVER";
	queue = openQueue(path, O_CREAT | O_RDONLY);
	
	receiver(path);

	return 0;
}