#include <stdlib.h>
#include <stdio.h>
#include "header.h"
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>


fifo* openQueue(){
	int des = shm_open("/fifo", O_RDWR, 0);
	fifo *result = (fifo*)mmap(NULL, sizeof(fifo), PROT_READ, MAP_SHARED, des, 0);
	int size = result->length * sizeof(long) + sizeof(fifo);
	munmap(result, sizeof(fifo));
	result = (fifo*)mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, des, 0);
	return result;
}

sem_t **openSemaphores(){
	sem_t **semaphores = malloc(sizeof(sem_t*) * 5);
	semaphores[CORRIDOR] = sem_open("/CORRIDOR", O_RDWR );
	semaphores[SLEEPING_BARBER] = sem_open("/SLEEPING_BARBER", O_RDWR );
	semaphores[WAKE_BARBER] = sem_open("/WAKE_BARBER", O_RDWR );
	semaphores[BARBER_CHAIR] = sem_open("/BARBER_CHAIR", O_RDWR );
	semaphores[END_OF_CUTTING] = sem_open("/END_OF_CUTTING", O_RDWR );
	return semaphores;
}

long *openPidBufer(){
	int des = shm_open("/pidBufer", O_RDWR, 0);
	return (long*)mmap(NULL, sizeof(long), PROT_READ | PROT_WRITE, MAP_SHARED, des, 0);
}


int main(int args, char *argv[]){
	if(args < 3)	return 0;

	int N = atoi(argv[1]);
	int S = atoi(argv[2]);

	for(int i = 0;i < N;i++){
		if(fork() == 0)	break;
	}

	fifo *queue = openQueue();

	sem_t **semaphores = openSemaphores();

	long *pidBufer = openPidBufer();

	for(int i = 0;i < S;){
		sem_wait(semaphores[CORRIDOR]);

		int buf;
		sem_getvalue(semaphores[SLEEPING_BARBER], &buf);
		if(buf == 0){
			*pidBufer = getpid();
			sem_post(semaphores[SLEEPING_BARBER]);
			sem_post(semaphores[WAKE_BARBER]);
			sem_post(semaphores[CORRIDOR]);
			sem_post(semaphores[BARBER_CHAIR]);
			sem_wait(semaphores[END_OF_CUTTING]);
			i++;
		} else {
			if(queue->numberElements < queue->length){

				addElement(queue, getpid());

				char *path = "/";
				char *charPid = Itoa(getpid(), calloc(16, 1), 10);
				strcat(path, charPid);
				sem_t *sem = sem_open(path, O_RDWR | O_CREAT, 0600, 0);

				sem_post(semaphores[CORRIDOR]);

				sem_wait(sem);

				sem_close(sem);
				sem_unlink(path);

				sem_post(semaphores[BARBER_CHAIR]);
				sem_wait(semaphores[END_OF_CUTTING]);
				i++;
			}
		}
	}
	return 0;
}