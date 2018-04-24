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

int queues[MAX_CLIENT_NUMBER];
int clients;
int queue;

void clearMsgbuf(struct msgbuf *msgbuf){
	memcpy(msgbuf->mtext, "", MSGBUF_SIZE);
}

void addClient(struct msgbuf *msgbuf) {
	int keyNumber = atoi(msgbuf->mtext);
	int candidateQueue = msgget(keyNumber, 0);
	if(candidateQueue == -1) {
		printf("Cannot open client queue\n");
	} else {
		if(clients < MAX_CLIENT_NUMBER){
			msgbuf -> mtype = 1;
			char *buf = malloc(128);
			strcpy(msgbuf->mtext, Itoa(clients, buf, 10));

			queues[clients] = candidateQueue;
			clients++;
			free(buf);
		} else {
			msgbuf->mtype = 2;
		}
		if(msgsnd(candidateQueue, msgbuf, MSGBUF_SIZE, 0) == -1)
			printf("Error during sending...\n");
		msgbuf ->mtype = 1;
	}
}

void mirror(struct msgbuf *msgbuf, int receivedChars){
	//who send
	char *buf = malloc(MSGBUF_SIZE - 16);
	memcpy(buf, msgbuf->mtext, 16);
	int clientID = atoi(buf);
	//what
	int textLen = strlen(msgbuf->mtext) - 16;
	memcpy(buf, msgbuf->mtext + 16, textLen);
	//mirror
	char c;
	for(int i = 0, j = textLen - 1; i < j; i++, j--){	//miejsce błędogenne
		c = buf[i];
		buf[i] = buf[j];
		buf[j] = c;
	}
	//send
	clearMsgbuf(msgbuf);
	memcpy(msgbuf->mtext, buf, textLen);
	msgsnd(queues[clientID], msgbuf, MSGBUF_SIZE, 0, 0);
	free(buf);
}

void sendTime(struct msgbuf *msgbuf){
	//who send
	char *buf = malloc(MSGBUF_SIZE - 16);
	memcpy(buf, msgbuf->mtext, 16);
	int clientID = atoi(buf);
	//get date
	fread(buf, 1, MSGBUF_SIZE - 16, popen("date", "r"));
	//send
	strcpy(msgbuf->mtext, buf);
	msgsnd(queues[clientID], msgbuf, MSGBUF_SIZE, 0, 0);
	free(buf);
}

void calc(struct msgbuf *msgbuf, int receivedChars) {
	char *buf = malloc(MSGBUF_SIZE - 16);
	memcpy(buf, msgbuf->mtext, 16);
	int clientID = atoi(buf);
	free(buf);

	commandLine *expression = processLineToCommandLine(msgbuf -> mtext + 16);

	int a = atoi(expression->array[0]);
	int b = atoi(expression->array[2]);
	char sign = expression->array[1][0];
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

	cleanCommandLine(expression);

	buf = malloc(32);

	strcpy(msgbuf->mtext, Itoa(result, buf, 10));
	msgsnd(queues[clientID], msgbuf, MSGBUF_SIZE, 1);
	free(buf);
}


void receiver(int queue){
	struct msgbuf *msgbuf = malloc(sizeof(struct msgbuf) + MSGBUF_SIZE);
	int receivedChars;
	int flags = 0;
	while((receivedChars = msgrcv(queue, msgbuf, MSGBUF_SIZE, 0, 0 | flags)) != -1){
		
		if(receivedChars == -1) {
			printf("Cannot read from queue\n");
			exit(1);
		}

		if(msgbuf -> mtype == ADD_CLIENT){
			addClient(msgbuf);
		}

		if(msgbuf -> mtype == END) {
			flags = IPC_NOWAIT;
		}

		if(msgbuf -> mtype == MIRROR) {
			mirror(msgbuf, receivedChars);
		}

		if(msgbuf -> mtype == CALC) {
			calc(msgbuf, receivedChars);
		}

		if(msgbuf -> mtype == TIME) {
			sendTime(msgbuf);
		}

	}

	quit(0);
}

void quit(int a){
	removeQueue(queue);
	exit(0);
}

int main(int args, char *argv[]) {
	clients = 0;
	char *homedir = getenv("HOME");
	key_t key = ftok(homedir, PROJECT_NUMBER);
	signal(SIGINT, &quit);
	queue = createQueue(key);
	receiver(queue);
	
	return 0;
}