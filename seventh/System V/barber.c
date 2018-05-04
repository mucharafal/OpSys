#define _POSIX_C_SOURCE 20000000

//#define VERBOSE 1

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/sem.h>
//queue
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "header.h"

int semaphores;
int queue;

void handler(int a){
	semctl(semaphores, 0, IPC_RMID);
	msgctl(queue, IPC_RMID, NULL);
	exit(0);
}

int main(int args, char *argv[]){
	/*
	arguments: ./barber N
	*/
	if(args < 2)	return 0;

	int N = atoi(argv[1]);

	signal(SIGINT, &handler);

	key_t key = ftok(".", 's');
	semaphores = semget(key, 7 + N, 0600 | IPC_CREAT);
	for(int i = 0;i < 7 + N;i++ )	semctl(semaphores, i, SETVAL, 0);

	semctl(semaphores, LIMIT_WAITING_CLIENTS, SETVAL, N);

	key = ftok("./barber.c", 's');
    queue = msgget(key, 0600 | IPC_CREAT);

	//Open!
    struct sembuf sops[1];
	sops[0].sem_num = CORRIDOR;
	sops[0].sem_op = 1;
	sops[0].sem_flg = 0;
	semop(semaphores, sops, 1);
	sops[0].sem_num = WAKE_BARBER;
	sops[0].sem_op = -1;
	sops[0].sem_flg = 0;
	semop(semaphores, sops, 1);

	#ifdef VERBOSE
	printf("Open!\n");
	#endif
	while(1) {
		struct msgbuf *msgbuf;
		msgbuf = malloc(sizeof(struct msgbuf));
		msgrcv(queue, msgbuf, 0, 0, 0);
		long pid = msgbuf -> mtype;
		
		free(msgbuf);
		#ifdef VERBOSE
		printf("Pid: %li\n", pid);

		printTime("Barber, before cutting");
		#endif
		goIn(semaphores, BARBER_CHAIR);
		#ifdef VERBOSE
		printTime("Barber, during cutting");
		#endif
		incSemaphore(semaphores, END_OF_CUTTING);
		#ifdef VERBOSE
		printTime("Barber, after cutting");
		#endif

		goIn(semaphores, CORRIDOR);
		#ifdef VERBOSE
		printTime("Barber, corridor");
		#endif
		int waitingCustomers = semctl(semaphores, WAITING_CLIENTS, GETVAL);
		#ifdef VERBOSE
		printf("Waiting clients: %i\n", waitingCustomers);
		#endif
		if(waitingCustomers > 0){
			//waiting room
			#ifdef VERBOSE
			printf("Taking from waiting room\n");
			#endif
			incSemaphore(semaphores, WAITING_ROOM_QUEUE);
			incSemaphore(semaphores, CORRIDOR);
			#ifdef VERBOSE
			printf("Barber: end of cutting\n");
			#endif
		} else {
			//release corridor and go sleep
			#ifdef VERBOSE
			printf("Sleep, yea...\n");
			#endif
			goIn(semaphores, SLEEPING_BARBER);
			incSemaphore(semaphores, CORRIDOR);
			goIn(semaphores, WAKE_BARBER);
		}
		#ifdef VERBOSE
		printTime("Barber, after choosing client");
		#endif
	}
    return 0;
}