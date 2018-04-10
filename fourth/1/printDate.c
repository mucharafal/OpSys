#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

int numberOfInvoke;
void handler(int signno);

void wait(){
	struct timespec ts;
	ts.tv_sec = 0;
	ts.tv_nsec = (1000 * 10); //10ms
	while(numberOfInvoke) {
		nanosleep(&ts, NULL);
	}
}

void handler(int signno){
	if(signno == -1){
		numberOfInvoke = 0;
	} else {
		printf("\n");
		if(numberOfInvoke == 0){
			printf("Oczekuję na CTRL+Z - kontynuacja albo CTR+C \n");
			numberOfInvoke = 1;
			//to avoid 100% using core
		} else {
			numberOfInvoke = 0;
		}
	}
}

void timeToSayGoodbye(int a){
	printf("\nBye!\n");
	exit(0);
}

int main(){
	//initialise handler
	handler(-1);
	signal(SIGTSTP, &handler);

	struct sigaction act;
	act.sa_handler = &timeToSayGoodbye;
	sigaction(SIGINT, &act, NULL);

	time_t t;
	while(1) {
		if(numberOfInvoke == 0) {
			sleep(1);
			time(&t);
			printf("%s", ctime(&t));
		} else {
			wait();
		}
	}
	return 0;
}