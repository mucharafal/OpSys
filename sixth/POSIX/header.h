#ifndef HEADER_H
#define HEADER_H 1

#define PROJECT_NUMBER 1
#define MAX_CLIENT_NUMBER 1
#define BUFFERSIZE 128
#define MSGBUF_SIZE 128
#define ADD_CLIENT 1
#define MIRROR 2
#define CALC 3
#define TIME 4
#define END 5
char *Itoa(long int, char*, int);
int createQueue(key_t);
void removeQueue(int);
typedef struct {
	char **array;
	int arraySize;
}commandLine;
commandLine *processLineToCommandLine();
void cleanCommandLine(commandLine *c);
#endif