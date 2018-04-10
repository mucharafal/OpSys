#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


void handle(int a){
	return;
}

int main(){

	signal(SIGUSR2, &handle);
	srand(getpid());
	int waitTime = rand() % 11;

	sleep(waitTime);
	union sigval s;
	s.sival_int = 0;
	sigqueue(getppid(), SIGRTMIN, s);
	pause();

	#ifndef USE_USR1
	int p = SIGRTMAX - SIGRTMIN - 1;
	int signum = SIGRTMIN + 1 + (rand() % p);
	sigqueue(getppid(), signum, s);
	#else
	int p = SIGRTMAX - SIGRTMIN;
	int signum = SIGRTMIN + (rand() % p);
	sigqueue(getppid(), signum, s);
	#endif

	return waitTime; 
}