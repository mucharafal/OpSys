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

union semun {
    int 			val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};


int main(int args, char *argv[]){
	/*
	arguments: ./barber N
	*/
	if(args < 2)	return 0;

	int N = atoi(argv[1]);

	key_t key = ftok("./GoInside", 's');
	int semGoInside = semget(key, 1, 0600 | IPC_CREAT);
	semctl(semGoInside, 0, SETVAL, N + 1);

	key = ftok("./Corridor", 's');
    int semCorridor = semget(key, 1, 0600 | IPC_CREAT);
	semctl(semCorridor, 0, SETVAL, 1);
	
    key = ftok("./SleepingBarber", 's');
    int semSleepingBarber = semget(key, 1, 0600 | IPC_CREAT);
	semctl(semSleepingBarber, 0, SETVAL, 10);
	
	//N waiting and one cut

    struct sembuf sops[1];
	sops[0].sem_num = 0;
	sops[0].sem_op = 1;
	sops[0].sem_flg = 0;
	semop(semGoInside, sops, 1);

    printf("Semaphore up, I can go!\n");
    return 0;
}