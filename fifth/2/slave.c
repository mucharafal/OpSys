#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int main(int args, char *argv[]){
	if(args > 2) {
		FILE *pipeHandler = fopen(argv[1], "w");
		
		int N = atoi(argv[2]);
		char *buffer = malloc(128);

		for(int i = 0 ; i < N ; i++ ){
			FILE *inputDate = popen("date", "r");
			fread(buffer, 128, 1, inputDate);
			pclose(inputDate);
			fprintf(pipeHandler, "%li ", getpid());
			fwrite(buffer, 128, 1, pipeHandler);
			sleep((rand()%4) + 2);
		}

		fclose(pipeHandler);
	}
	return 0;
}