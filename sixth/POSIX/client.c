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
#include <mqueue.h>
#include <string.h>
#include "header.h"
#include <errno.h>


int connectWithServer(int *serverQueue, int *clientQueue, int *clientID){
	int result;

	//opening client queue
	char *queuePath = calloc(12, 1);
	strcpy(queuePath, "/");

	char *buffer = malloc(11);
	strcat(queuePath, Itoa(getpid(), buffer, 10));

	free(buffer);

	*clientQueue = openQueue(queuePath, O_CREAT | O_RDONLY);

	if(*clientQueue != -1) 
		printf("Opened\n");
	else 
		fprintf(stderr, "Error opening file: %s\n", strerror( errno ));
	//opening server queue
	strcpy(queuePath, "/SERVER");

	*serverQueue = openQueue(queuePath, O_WRONLY);

	if(*serverQueue != -1) 
		printf("Opened\n");
	else 
		fprintf(stderr, "Error opening file: %s\n", strerror( errno ));

	free(queuePath);
	//preparing to register on server
	char *buf = malloc(64);
	Itoa(getpid(), buf, 10);

	char *message = calloc(MSGBUF_SIZE, 1);
	strcpy(message, "ADD ");
	strcat(message, buf);

	if(mq_send(*serverQueue, message, MSGBUF_SIZE, 0) == -1) {
		fprintf(stderr, "Error sending: %s\n", strerror( errno ));
	} else {

		strclear(message);
		if(mq_receive(*clientQueue, message, MSGBUF_SIZE, NULL) == -1) {
			printf("Error occueeeed...\n");
		}

		commandLine *expression = processLineToCommandLine(message);

		if(strcmp(expression->array[0], "NO") != 0) {
			printf("Connected\n");
			*clientID = atoi(expression->array[0]);
			result = 1;
		}
		else {
			printf("Cannot connect, server is full\n");
			result = 0;
		}
	}

	free(buf);
	return result;
}

void prepareMsgbufToSend(char *message, int clientID){
	strclear(message);
	Itoa(clientID, message, 10);
	strcat(message, " ");
}
void processFile(char *fileName, int server, int client, int clientID){

	char *buf = malloc(BUFFERSIZE);
	FILE* file = fopen(fileName, "r");
	int lineLength;
	while(fgets(buf, BUFFERSIZE, file)){
		int wasContentInLine = 1;
		commandLine *line = processLineToCommandLine(buf);
		if(line->array[0] == 0)	continue;

		char *message = calloc(MSGBUF_SIZE, 1);
		prepareMsgbufToSend(message, clientID);
		
		strcat(message, buf);

		mq_send(server, message, MSGBUF_SIZE, 0);
		if(strcmp(line->array[0], "END") != 0 && wasContentInLine){
			strclear(message);
			mq_receive(client, message, MSGBUF_SIZE, 0);
			printf("Received: %s\n", message);
		}

		cleanCommandLine(line);
		free(message);
	}

	fclose(file);
	free(buf);
}

int main(int args, char *argv[]) {
	int server, client, clientID;

	if(connectWithServer(&server, &client, &clientID))
		processFile(argv[1], server, client, clientID);

	char *queuePath = calloc(12, 1);
	strcpy(queuePath, "/");

	char *buffer = malloc(11);
	strcat(queuePath, Itoa(getpid(), buffer, 10));

	free(buffer);

	removeQueue(queuePath);
	return 0;
}