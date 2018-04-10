#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

#define USING_SCRIPT_CHILD 1

int numberOfInvoke;
__pid_t childPid = -1;

void handler(int signno);
void killChild();
void createChild();

void wait(){
	while(1) {
		pause();
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
			killChild();
			childPid = -1;
		} else {
			createChild();
			numberOfInvoke = 0;
		}
	}
}

void timeToSayGoodbye(int a){
	killChild();
	printf("\nOdebrano sygnał SIGINT\n");
	exit(0);
}

void createChild(){
	childPid = fork();
	if(childPid == 0){
		#ifdef USING_SCRIPT_CHILD
		execl("./childBash", "childBash", NULL);
		#else
		execl("./printDateChild", "printDateChild", NULL);
		#endif
	}
}

void killChild() {
	if(childPid != -1)
		kill(childPid, SIGKILL);
}

int main(){
	//initialise handler
	handler(-1);
	signal(SIGTSTP, &handler);

	struct sigaction act;
	act.sa_handler = &timeToSayGoodbye;
	sigaction(SIGINT, &act, NULL);

	createChild();
	wait();
	
	return 0;
}