#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#define BUFFERSIZE 	4096

typedef struct {
	char **array;
	int arraySize;
}commandLine;

typedef struct{
	commandLine **array;
	int arraySize;
}commandLinePipeline;

void cleanCommandLine(commandLine *c){
	int i;
	for(int i = 0; i < c->arraySize ;i++)
		free(c->array[i]);
	free(c->array);
}

void cleanCommandLinePipeline(commandLinePipeline *p){
	int i;
	for(int i = 0; i < p->arraySize ;i++)
		cleanCommandLine((p->array[i]));
	free(p->array);
}

commandLine* processLineToCommandLine(char *line){

	int argumentsNumber = 0;
	int resultSize = 1;

	char **result = malloc(sizeof(char*));
	char *BUFFER = malloc(BUFFERSIZE);
	char *temp = line;

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

commandLinePipeline* processLineToCommandLineWithPipeline(char *line){

	int processes = 1;
	commandLine **result = malloc(sizeof(commandLine*));

	char *BUFFER = calloc(BUFFERSIZE, sizeof(char));
	int bufferCounter = 0;

	for(int i = 0;line[i] != 0;i++){
		if(line[i] != '|'){
			BUFFER[bufferCounter] = line[i];
			bufferCounter++;
		} else {
			BUFFER[bufferCounter] = 0;
			result[processes - 1] = processLineToCommandLine(BUFFER);
			
			processes++;
			result = realloc(result, sizeof(void*) * processes);

			bufferCounter = 0;
		}
	}
	BUFFER[bufferCounter] = 0;
	result[processes - 1] = processLineToCommandLine(BUFFER);

	commandLinePipeline *new = malloc(sizeof(commandLinePipeline));
	new -> array = result;
	new -> arraySize = processes;
}

void execCommandLine(commandLine *parsedLine) {

	int errorStatus = 1;

	if(parsedLine->array[0][0] == '/' || parsedLine->array[0][0] == '.'){
		errorStatus = execv(parsedLine->array[0], parsedLine->array);
	} else {
		//using path
		errorStatus = execvp(parsedLine->array[0], parsedLine->array);
	}

	if(errorStatus < 0){
		printf("Error during exec occured\n");
	}
}

void runLineWithPipes(commandLinePipeline *line){
	if(line -> arraySize == 1){
		execCommandLine(line -> array[0]);
	} else {
		for(int i = 0;i < line -> arraySize - 1;i++) {

			int fd[2];
			pipe(fd);
			pid_t pid = fork();
			if(pid == 0){
				close(fd[1]);
				dup2(fd[0], 0);
				
			} else {
				close(fd[0]);
				dup2(fd[1], 1);

				commandLine *parsedLine = line -> array[i];
				execCommandLine(parsedLine);
				wait(NULL);
			}
		}
		commandLine *parsedLine = line -> array[line -> arraySize - 1];
		execCommandLine(parsedLine);
	}
}

void execLine(char *line){
	pid_t pid = fork();
	if(pid == 0){
		runLineWithPipes(processLineToCommandLineWithPipeline(line));
	} else {
		wait(NULL);
	}
}

int main(int args, char *argv[]){
	setbuf(stdout, NULL);
	if(args > 1){

		//open file and processing
		FILE* file = fopen(argv[1], "r");
		if(file == NULL){
			printf("Cannot open file.\n");
			return 1;
		}

		char line[BUFFERSIZE];
		while(fgets(line, BUFFERSIZE, file)){
			execLine(line);
		}
		printf("End of program...\n");
	}
	return 0;
}