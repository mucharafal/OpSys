#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define PRINT_NUMBER_RECEIVED_SIGNALS 1

int receivedSignals;

void handle(int a){
	receivedSignals++;

	#ifdef PRINT_NUMBER_RECEIVED_SIGNALS
	printf("Child %lu received %i signals\n", getpid(), receivedSignals);
	#endif
	
	kill(getppid(), a);
	return;
}

void endHandle(int a){
	exit(0);
}

int main(){

	receivedSignals = 0;
	
	signal(SIGUSR1, &handle);
	signal(SIGRTMIN, &handle);

	signal(SIGUSR2, &endHandle);
	signal(SIGRTMAX, &endHandle);

	kill(getppid(), SIGRTMIN);

	while(1){
		pause();
	}
}