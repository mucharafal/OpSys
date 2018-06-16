#ifndef HEADER_H
#define HEADER_H 1

#define PROJECT_NUMBER 1
#define MAX_CLIENT_NUMBER 1
#define BUFFERSIZE 64
#define MSGBUF_SIZE 64
#define ADD_CLIENT 1
#define MIRROR 2
#define CALC 3
#define TIME 4
#define END 5
#define WAIT_TIME 10
char *Itoa(long int, char*, int);
int openQueue(char *queuePath, int openMode);
void removeQueue(char *name);
typedef struct {
	char **array;
	int arraySize;
}commandLine;
commandLine *processLineToCommandLine();
void cleanCommandLine(commandLine *c);
void strclear(char *s);
#endif