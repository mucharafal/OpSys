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

int createQueue(key_t key){
	int queue = msgget(key, IPC_CREAT | 0640 | IPC_PRIVATE);
	if(queue == -1) {
		printf("Cannot create queue\n");
		exit(1);
	}
	return queue;
}

void removeQueue(int queue) {
	msgctl(queue, IPC_RMID, NULL);
}

commandLine* processLineToCommandLine(char *line){

	int argumentsNumber = 0;
	int resultSize = 1;

	char **result = calloc(1, sizeof(char*));
	char *BUFFER = malloc(BUFFERSIZE);
	char *temp = malloc(strlen(line) + 1);
	strcpy(temp, line);

	while(BUFFER = strsep(&temp, " \n")){

		if(strcmp(BUFFER, "") == 0)		continue;
		if(strcmp(BUFFER, " ") == 0)	continue;
		if(strcmp(BUFFER, "\n") == 0)	continue;

		int size = strlen(BUFFER);
		size++;
		char *argument = malloc(sizeof(char) * size);

		strcpy(argument, BUFFER);

		argumentsNumber++;
		if(resultSize < argumentsNumber){
			result = realloc(result, sizeof(char*) * argumentsNumber);
			resultSize = argumentsNumber;
		}
		
		result[argumentsNumber - 1] = argument;
	}
	result = realloc(result, sizeof(char*) * (resultSize + 1));
	result[argumentsNumber] = NULL;

	commandLine *res = malloc(sizeof(commandLine));
	res->array = result;
	res->arraySize = resultSize;
	return res;
}

void cleanCommandLine(commandLine *c){
	int i;
	for(int i = 0; i < c->arraySize ;i++)
		free(c->array[i]);
	free(c->array);
}