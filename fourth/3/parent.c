#define __USE_XOPEN_EXTENDED 1
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>

void waitingHandler(int a);
void sentSignals(int, int, int);
__pid_t *childPids;
int children;
int receivedSignals;

void wariant1(int L){
	sentSignals(L, SIGUSR1, SIGUSR2);
}

void wariant3(int L){
	sentSignals(L, SIGRTMIN, SIGRTMAX);
}

void sentSignals(int numberOfSignals, int workSignal, int terminateSignal){
	/*blocking signals which will be sent, because kill(0, _) send signal to itself too*/
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, workSignal);
	sigaddset(&set, terminateSignal);
	sigprocmask(SIG_BLOCK, &set, NULL);
	/*sending signals*/
	for(int i = 0;i < numberOfSignals;i++){
		kill(0, workSignal);
		#ifdef PRINT_NUMBER_SENT_SIGNALS
		printf("Received %i signals\n", children * i);
		#endif
	}
	kill(0, terminateSignal);
}

void wariant2(int L, int N){
	//blocking SIGUSR2, because of using kill(0, _)
	sigset_t set;
	sigemptyset(&set);
	sigaddset(&set, SIGUSR2);
	sigprocmask(SIG_BLOCK, &set, NULL);

	//handler for
	struct sigaction sa;
	sa.sa_handler = &waitingHandler;
	sa.sa_flags = 0;

	sigaction(SIGUSR1, &sa, NULL);

	for(int i = 0;i < L;i++){
		#ifdef PRINT_NUMBER_SENT_SIGNALS
		printf("Sent and received %i signals SIGUSR1\n", children * i);
		#endif
		for(int j = 0;j < children;j++){
			receivedSignals = 0;
			kill(childPids[j], SIGUSR1);
			while(receivedSignals == 0) ;
		}
	}
	kill(0, SIGUSR2);
}

void waitingHandler(int a){
	receivedSignals++;
}

void createChildren(int number){
	for(int i = 0; i < number; i++){
		receivedSignals = 0;
		__pid_t pid = fork();
		if(pid == 0){
			if(execl("./child", "child", NULL) == -1)
				printf("Cant start child\n");
		} else {
			childPids[i] = pid;
			while(receivedSignals == 0) ;
		}
	}
}

void handlersInit(){
	signal(SIGRTMIN, &waitingHandler);
}

int main(int args, char *argv[]){
	/*
	Aruments:
	./parent Children Mode N
	Childern - number of children
	Mode - number of mode, 1,2,3
	N - number signals to send
	*/
	if(args > 3){
		handlersInit();

		children = atoi(argv[1]);
		int mode = atoi(argv[2]);
		int N = atoi(argv[3]);
		
		childPids = malloc(sizeof(__pid_t) * children);

		createChildren(children);
		switch(mode){
		case 1:
			wariant1(N);
			break;
		case 2:
			wariant2(N, children);
			break;
		case 3:
			wariant3(N);
			break;
		}
	}
	return 0;
}