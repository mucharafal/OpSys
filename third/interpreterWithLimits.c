#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/times.h>
#include <sys/resource.h>
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
	
	if(args > 3){
		//parsing command line arguments
		struct rlimit memoryLimit;
		getrlimit(RLIMIT_AS, &memoryLimit);
		memoryLimit.rlim_cur = atoi(argv[2]);
		struct rlimit timeLimit;
		getrlimit(RLIMIT_CPU, &timeLimit);
		timeLimit.rlim_cur = atoi(argv[3]);

		//preparing to measure time
		clock_t startTime;
		struct tms cpuStartTime;
		startTime = times(&cpuStartTime);

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
				//child process
				//setting limits
				int errorSettingLimits = 0;
				if(setrlimit(RLIMIT_CPU, &timeLimit) != 0) {
					printf("Cannot set setrlimit CPU\n");
				}
				if(setrlimit(RLIMIT_AS, &memoryLimit) != 0) {
					printf("Cannot set setrlimit memory\n");
				}
				//exec
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
				//parent process
				int exitCode;
				wait(&exitCode);

				//time count
				clock_t endTime;
    			struct tms cpuEndTime;
    			endTime = times(&cpuEndTime);

				printf("Real Time: %i, User Time %i, System Time %i\n",
					(endTime - startTime),
					(cpuEndTime.tms_utime - cpuStartTime.tms_utime),
					(cpuEndTime.tms_stime - cpuStartTime.tms_stime));

				cpuStartTime = cpuEndTime;
				startTime = endTime;

				if(exitCode != 0){
					printf("Process exit with error code: %i\n%s", exitCode, strerror(exitCode));
					return 2;
				}

				cleanCommandLine(parsedLine);
			}
		}
	} else {
		printf("Too few arguments.\n");
	}
	return 0;
}