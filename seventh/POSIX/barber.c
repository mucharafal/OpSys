#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "header.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

fifo *createFifo(int N){
	int des = shm_open("/fifo", O_RDWR | O_CREAT, 0600);
	int size = sizeof(fifo) + sizeof(long) * N;
	ftruncate(des, size);
	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, des, 0);
	fifo *result = (fifo*)addr;
	result->length = N;
	result->numberElements = 0;
	result->firstElement = 0;
	return result;
}

sem_t **createSemaphores(){
	sem_t **semaphores = malloc(sizeof(sem_t*) * 5);
	semaphores[CORRIDOR] = sem_open("/CORRIDOR", O_RDWR | O_CREAT, 0600, 0);
	semaphores[SLEEPING_BARBER] = sem_open("/SLEEPING_BARBER", O_RDWR | O_CREAT, 0600, 0);
	semaphores[WAKE_BARBER] = sem_open("/WAKE_BARBER", O_RDWR | O_CREAT, 0600, 0);
	semaphores[BARBER_CHAIR] = sem_open("/BARBER_CHAIR", O_RDWR | O_CREAT, 0600, 0);
	semaphores[END_OF_CUTTING] = sem_open("/END_OF_CUTTING", O_RDWR | O_CREAT, 0600, 0);
	return semaphores;
}

long *createPidBufer(){
	int des = shm_open("/pidBufer", O_RDWR | O_CREAT, 0600);
	int size = sizeof(long);
	ftruncate(des, size);
	void *addr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, des, 0);
	long *result = (long*)addr;
	*result = 0;
	return result;
} 

int main(int argv, char *args[]){
	if(argv < 2)	return 0;
	//N- length of queue
	int N = atoi(args[1]);

	fifo *queue = createFifo(N);

	long *pid = createPidBufer();

	sem_t **semaphores = createSemaphores();

	//open!
	sem_post(semaphores[CORRIDOR]);
	sem_wait(semaphores[WAKE_BARBER]);

	while(1){

		printf("Pid: %li\n", *pid);

		sem_wait(semaphores[BARBER_CHAIR]);

		sem_post(semaphores[END_OF_CUTTING]);

		sem_wait(semaphores[CORRIDOR]);
		if(queue->numberElements <= 0){
			printf("Sleep\n");
			//sleep
			sem_wait(semaphores[SLEEPING_BARBER]);
			sem_post(semaphores[CORRIDOR]);
			sem_wait(semaphores[WAKE_BARBER]);
		} else {
			printf("Taking from queue\n");
			long pidFromQueue = takeElement(queue);
			char *path = "/";
			char *charPid = Itoa(pidFromQueue, calloc(16, 1), 10);
			strcat(path, charPid);
			sem_t *sem = sem_open(path, O_RDWR);
			sem_post(sem);
			*pid = pidFromQueue;
			sem_close(sem);
			sem_post(semaphores[CORRIDOR]);
		}
	}

	return 0;
}