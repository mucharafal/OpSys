#define _POSIX_C_SOURCE 20000000

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


int main(int args, char *argv[]){
	/*
	arguments: ./barber N
	*/
	if(args < 2)	return 0;

	int N = atoi(argv[1]);

	//signal(SIGTERM, &handler);

	key_t key = ftok(".", 's');
	int semaphores = semget(key, 7 + N, 0600 | IPC_CREAT);
	for(int i = 0;i < 7 + N;i++ )	semctl(semaphores, i, SETVAL, 0);

	semctl(semaphores, LIMIT_WAITING_CLIENTS, SETVAL, N);

	key = ftok("./barber.c", 's');
    int queue = msgget(key, 0600 | IPC_CREAT);

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

	printf("Open!\n");

	while(1) {
		struct msgbuf *msgbuf;
		msgbuf = malloc(sizeof(struct msgbuf));
		msgrcv(queue, msgbuf, 0, 0, 0);
		long pid = msgbuf -> mtype;
		
		free(msgbuf);

		printf("Pid: %li\n", pid);

		printTime("Barber, before cutting");
		goIn(semaphores, BARBER_CHAIR);
		printTime("Barber, during cutting");
		incSemaphore(semaphores, END_OF_CUTTING);
		printTime("Barber, after cutting");

		goIn(semaphores, CORRIDOR);
		printTime("Barber, corridor");
		int waitingCustomers = semctl(semaphores, WAITING_CLIENTS, GETVAL);
		printf("Waiting clients: %i\n", waitingCustomers);
		if(waitingCustomers > 0){
			//waiting room
			printf("Taking from waiting room\n");
			incSemaphore(semaphores, WAITING_ROOM_QUEUE);
			goIn(semaphores, WAITING_CLIENTS);
			incSemaphore(semaphores, CORRIDOR);
			printf("Barber: end of cutting\n");
		} else {
			//release corridor and go sleep
			printf("Sleep, yea...\n");
			goIn(semaphores, SLEEPING_BARBER);
			incSemaphore(semaphores, CORRIDOR);
			goIn(semaphores, WAKE_BARBER);
		}
		printTime("Barber, after choosing client");
	}

    printf("Semaphore up, I can go!\n");
    return 0;
}