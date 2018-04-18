#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

int main(int args, char *argv[]){
	/*arguments:
	argv[1] - pipe name
	argv[2] - number messages
	*/
	if(args > 2) {
		printf("Slave pid: %li\n", getpid());
		
		int N = atoi(argv[2]);
		char *buffer = malloc(128);

		for(int i = 0 ; i < N ; i++ ){
			FILE *inputDate = popen("date", "r");
			fread(buffer, 128, 1, inputDate);
			pclose(inputDate);

			FILE *pipeHandler = fopen(argv[1], "w");

			fprintf(pipeHandler, "%li ", getpid());
			fwrite(buffer, 128, 1, pipeHandler);

			fclose(pipeHandler);

			sleep((rand()%4) + 2);
		}
		
	}
	return 0;
}