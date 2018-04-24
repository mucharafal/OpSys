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
#include "header.h"


int connectWithServer(int *serverQueue, int *clientQueue, int *clientID){
	int result;
	char *homedir = getenv("HOME");
	key_t key = ftok(homedir, PROJECT_NUMBER);
	*serverQueue = msgget(key, 0);
	if(*serverQueue != -1) ;
	else 
		printf("Cannot open\n");
	
	key = ftok(homedir, getpid());

	*clientQueue = createQueue(key);

	char *buf = malloc(64);
	Itoa(key, buf, 10);

	struct msgbuf *msgbuf = malloc(sizeof(struct msgbuf) + MSGBUF_SIZE);
	strcpy(msgbuf -> mtext, buf);
	msgbuf -> mtype = 1;

	if(msgsnd(*serverQueue, msgbuf, MSGBUF_SIZE, 0) == -1) {
		printf("Cant send\n");
	} else {
		printf("Waiting for responce...\n");
		if(msgrcv(*clientQueue, msgbuf, MSGBUF_SIZE, 0, 0) == -1) {
			printf("Error occured...\n");
		}
		printf("Received\n");
		if(msgbuf->mtype == 1) {
			printf("Connected\n");
			*clientID = atoi(msgbuf->mtext);
			result = 1;
		}
		else {
			printf("Cannot connect, server is full\n");
			result = 0;
		}
	}

	free(msgbuf);
	free(buf);
	return result;
}

void prepareMsgbufToSend(struct msgbuf *msgbuf, int clientID){
	char *buf = malloc(32);
	strcpy(msgbuf-> mtext, Itoa(clientID, buf, 10));
	strcat(msgbuf->mtext, "                ");
	memcpy(msgbuf->mtext + 16, "", MSGBUF_SIZE - 16);
	free(buf);
}
void processFile(char *fileName, int server, int client, int clientID){
	struct msgbuf *msgbuf = malloc(sizeof(struct msgbuf) + MSGBUF_SIZE);
	msgbuf -> mtype = CALC;

	char *buf = malloc(BUFFERSIZE);
	FILE* file = fopen(fileName, "r");
	int lineLength;
	while(fgets(buf, BUFFERSIZE, file)){
		int wasContentInLine = 0;
		commandLine *line = processLineToCommandLine(buf);
		if(line->array[0] == 0)	continue;
		prepareMsgbufToSend(msgbuf, clientID);

		if(strcmp(line->array[0], "CALC") == 0){
			msgbuf -> mtype = CALC;
			buf[0] = 0;
			strcat(buf, line->array[1]);
			strcat(buf, " ");
			strcat(buf, line->array[2]);
			strcat(buf, " ");
			strcat(buf, line->array[3]);
			msgbuf->mtext[16] = 0;
			strcat(msgbuf->mtext + 16, buf);

			wasContentInLine = 1;
		}
		if(strcmp(line->array[0], "MIRROR") == 0){
			//dodać kontrolę długości
			strcpy(msgbuf ->mtext + 16, buf + 7);
			msgbuf->mtype = MIRROR; 

			wasContentInLine = 1;
		}
		if(strcmp(line->array[0], "DATE") == 0){
			msgbuf->mtype = TIME;

			wasContentInLine = 1;
		}
		if(strcmp(line->array[0], "END") == 0){
			msgbuf->mtype = END;

			wasContentInLine = 1;
		}
		msgsnd(server, msgbuf, MSGBUF_SIZE, 0);
		if(strcmp(line->array[0], "END") != 0 && wasContentInLine){
			msgrcv(client, msgbuf, MSGBUF_SIZE, 0, 0);
			printf("Received: %s\n", msgbuf->mtext);
		}

		cleanCommandLine(line);
	}

	

	fclose(file);
	free(buf);
	free(msgbuf);
}

int main(int args, char *argv[]) {
	int server, client, clientID;

	if(connectWithServer(&server, &client, &clientID))
		processFile(argv[1], server, client, clientID);

	removeQueue(client);
	return 0;
}