#define __USE_XOPEN_EXTENDED 1
#define PRINT_EXIT_STATUS 1
#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
/*
Makra:
USE_USR1 - używa SIGUSR1 do komunikacji dziecko - rodzic. Powoduje problemy
związane z brakiem kolejkowania sygnałów
Gdy nie zdefiniowane do komunikacji używane jest SIGRTMIN. Ten sygnał jest kolejkowany.
Wymaga zdefiniowania tego samego w dziecku i rodzicu.
PRINT_EXIT_STATUS - Gdy zdefiniowane program wypisuje pid i status zakończenia procesu dziecka.
TRACK_NEW_CHILD - gdy zdefiniowane powoduje wypisanie pid utworzonego procesu potomnego
RECEIVED_ASKS - liczba próśb od procesów potomnych
AGREE_SIGNAL - sygnały USR2 do procesów potomnych
RTSIGNAL - wypisywana jest informacja o sygnałach RT odebranych od dzieci

Argumenty:
./parent N M
N - liczba procesów potomnych
M - liczba oczekiwanych sygnałów od dzieci
*/

int N, M;

void waitForChildren(){
	int finishedChildren = N;
	while(finishedChildren){
		int status;
		__pid_t pid = wait(&status);
		if(pid != -1)	{
			finishedChildren--;

			#ifdef PRINT_EXIT_STATUS
			status /= 256;
			printf("%i exit status: %i, number: %i\n", pid, status, finishedChildren);
			#endif
		}
	}
}

void handleRTI(int signalNumber, siginfo_t *info, void *foo ){
	static int receivedSignals;
	static int *signals;
	static pid_t *pids;
	if(signalNumber < 0){
		if(signalNumber == -1){
			receivedSignals = 0;
			signals = malloc(sizeof(int) * N);
			pids = malloc(sizeof(pid_t) * N);
		} else {
			//-2
			for(int i = 0;i < N;i++)	printf("From %i: %i\n", pids[i], signals[i]);
		}
	} else {
		signals[receivedSignals] = signalNumber;
		pids[receivedSignals] = info->si_pid;
		receivedSignals++;
	}
}

void handleUSR1(int signalNumber, siginfo_t *info, void *foo){

	static pid_t *pids;
	static int receivedSignals;
	//init
	if(signalNumber == -1){
		if(M != 0){
			pids = malloc(sizeof(pid_t) * M);
		} else {
			pids = NULL;
		}
		receivedSignals = 0;
	} else {
		pid_t pid = info->si_pid;
		int sig = info->si_signo;

		#ifdef RECEIVED_ASKS
		printf("Received ask from: %li, number: %i\n", pid, receivedSignals);
		#endif

		if(receivedSignals < M){
			pids[receivedSignals] = pid;
		}
		receivedSignals++;
		if(pids != NULL && receivedSignals >= M){
			//sent them signal
			for(int i = 0;i < M;i++) {
				kill(pids[i], SIGUSR2);
				#ifdef AGREE_SIGNAL
				printf("SIGUSR2 to %li\n", pids[i]);
				#endif
			}
			free(pids);
			pids = NULL;
		} else {
			if(receivedSignals >= M) {
				kill(pid, SIGUSR2);
				#ifdef AGREE_SIGNAL
				printf("SIGUSR2 to %li\n", pid);
				#endif
			}
		}
	}
}

void killThemAll(int a){
	signal(SIGQUIT, SIG_IGN);
	kill(-getpid(), SIGQUIT);
}

void createChildren(){
	for(int i = 0;i < N;i++ ){
		__pid_t pid = fork();
		if(pid == 0) {
			execl("./child", "child", NULL);
		} 
		#ifdef TRACK_NEW_CHILD
		else{
			printf("Child pid: %lu\n", pid);
		}
		#endif
	}
}

void initHandlers(){
	handleRTI(-1, NULL, NULL);
	handleUSR1(-1, NULL, NULL);
}

int main(int args, char *argv[]){
	signal(SIGINT, &killThemAll);

	struct sigaction s;

	memset (&s, '\0', sizeof(s));
 
	/* The SA_SIGINFO flag tells sigaction() to use the sa_sigaction field, not sa_handler. */
	s.sa_flags = SA_SIGINFO;
	
	s.sa_sigaction = &handleUSR1;
	#ifndef USE_USR1
	sigaction(SIGRTMIN, &s, NULL);
	s.sa_sigaction = &handleRTI;
	for(int i = SIGRTMIN + 1;i <= SIGRTMAX;i++ ){
		sigaction(i, &s, NULL);
	}
	#else
	sigaction(SIGUSR1, &s, NULL);
	s.sa_sigaction = &handleRTI;
	for(int i = SIGRTMIN;i <= SIGRTMAX;i++ ){
		sigaction(i, &s, NULL);
	}
	#endif

	
	if(args > 2) {
		N = atoi(argv[1]);
		M = atoi(argv[2]);

		if(N < M)	return 1;

		initHandlers();

		createChildren();
		
		waitForChildren();

		#ifdef RTSIGNAL
		handleRTI(-2, NULL, NULL);
		#endif
		return 0;
	} else {
		return 1;
	}
}