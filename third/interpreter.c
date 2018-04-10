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

void cleanCommandLine(commandLine *c){
	int i;
	for(int i = 0; i < c->arraySize ;i++)
		free(c->array[i]);
	free(c->array);
}

commandLine* splitOnWhitespaces(char *line){

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

			commandLine *parsedLine = splitOnWhitespaces(line);

			__pid_t pid = fork();
			if(pid == 0){

				int errorStatus = 1;
				if(parsedLine->array[0][0] == '/' || parsedLine->array[0][0] == '.'){
					errorStatus = execv(parsedLine->array[0], parsedLine->array);
				} else {
					//using path
					errorStatus = execvp(parsedLine->array[0], parsedLine->array);
				}

				if(errorStatus < 0){
					printf("Error during exec occured\n");
					return 1;
				}
			} else {
				int exitCode;
				wait(&exitCode);
				if(exitCode != 0){
					printf("Process exit with error code: %i\n", exitCode);
					return 2;
				}

				cleanCommandLine(parsedLine);
			}
		}
	}
	return 0;
}