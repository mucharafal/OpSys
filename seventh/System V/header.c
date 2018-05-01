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

void goIn(int semaphores, int semaphore){
    struct sembuf sops[1];
    sops[0].sem_num = semaphore;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    semop(semaphores, sops, 1);
}

void incSemaphore(int semaphores, int semaphore){
    struct sembuf sops[1];
    sops[0].sem_num = semaphore;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    semop(semaphores, sops, 1);
}

void printTime(char *name){
	struct timespec time;
	clock_gettime(CLOCK_MONOTONIC, &time);
	printf("%s; pid: %li, time %i,%li sec\n", name, getpid(), time.tv_sec, time.tv_nsec / 1000);
}