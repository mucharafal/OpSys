#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>

FILE *pipeHandler;
void handler(int a){
	pclose(pipeHandler);
	exit(0);
}

int main(int args, char *argv[]){
	/*arguments:
	argv[1] - pipe name
	*/
	if(args > 1) {
		signal(SIGINT, &handler);
		mkfifo(argv[1], 0640);

		pipeHandler = fopen(argv[1], "r");
		
		char *buffer = malloc(128);
		buffer[0] = 1;
		while(1) {
			fread(buffer, 128, 1, pipeHandler);
			printf("%s", buffer);
		}
	}
}