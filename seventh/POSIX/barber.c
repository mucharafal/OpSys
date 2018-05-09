#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "header.h"
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

sem_t **semaphores;
int N;
fifo *queue;
long *pid;

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

void createSemaphores(){
	semaphores = malloc(sizeof(sem_t*) * 5);
	semaphores[CORRIDOR] = sem_open("/CORRIDOR", O_RDWR | O_CREAT, 0600, 0);
	semaphores[SLEEPING_BARBER] = sem_open("/SLEEPING_BARBER", O_RDWR | O_CREAT, 0600, 0);
	semaphores[WAKE_BARBER] = sem_open("/WAKE_BARBER", O_RDWR | O_CREAT, 0600, 0);
	semaphores[BARBER_CHAIR] = sem_open("/BARBER_CHAIR", O_RDWR | O_CREAT, 0600, 0);
	semaphores[END_OF_CUTTING] = sem_open("/END_OF_CUTTING", O_RDWR | O_CREAT, 0600, 0);
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

void handler(int a){
	sem_close(semaphores[CORRIDOR]);
	sem_unlink("/CORRIDOR");
	sem_close(semaphores[SLEEPING_BARBER]);
	sem_unlink("/SLEEPING_BARBER");
	sem_close(semaphores[WAKE_BARBER]);
	sem_unlink("/WAKE_BARBER");
	sem_close(semaphores[BARBER_CHAIR]);
	sem_unlink("/BARBER_CHAIR");
	sem_close(semaphores[END_OF_CUTTING]);
	sem_unlink("/END_OF_CUTTING");
	free(semaphores);
	int sizefifo = sizeof(fifo) + sizeof(long) * N;
	munmap(queue, sizefifo);
	shm_unlink("/fifo");
	munmap(pid, sizeof(long));
	shm_unlink("/pidBufer");
	exit(0);
}


int main(int argv, char *args[]){
	if(argv < 2)	return 0;
	//N- length of queue
	N = atoi(args[1]);

	queue = createFifo(N);

	pid = createPidBufer();

	createSemaphores();

	signal(SIGINT, &handler);

	//open!
	printTime("Opened");
	sem_post(semaphores[CORRIDOR]);
	sem_wait(semaphores[WAKE_BARBER]);
	printTime("First client...");
	while(1){

		printf("Pid: %li\n", *pid);

		sem_wait(semaphores[BARBER_CHAIR]);
		printTime("After taking the chair");
		sem_post(semaphores[END_OF_CUTTING]);
		printTime("After cutting");
		sem_wait(semaphores[CORRIDOR]);
		if(queue->numberElements <= 0){
			printTime("Sleep");
			//sleep
			sem_wait(semaphores[SLEEPING_BARBER]);
			sem_post(semaphores[CORRIDOR]);
			sem_wait(semaphores[WAKE_BARBER]);
			printTime("Woken");
		} else {
			printTime("Taking from queue");
			long pidFromQueue = takeElement(queue);
			char *path = calloc(18, 1);
			strcpy(path, "/");
			char *charPid = Itoa(pidFromQueue, calloc(16, 1), 10);
			strcat(path, charPid);
			printf("Waking client: %s\n", path);
			sem_t *sem = sem_open(path, O_RDWR);
			sem_post(sem);
			*pid = pidFromQueue;
			sem_close(sem);
			sem_post(semaphores[CORRIDOR]);
			free(path);
		}
	}

	return 0;
}