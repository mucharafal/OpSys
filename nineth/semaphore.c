#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <semaphore.h>

#define BUFER_SIZE 1024

char **array;
int read_pointer, write_pointer, elements_number, array_size;

pthread_t *producers;
pthread_t *consumers;

int verbose;

int writer_end;

sem_t *elements_to_read, *elements_to_write;
sem_t *read_pointer_semaphore, *write_pointer_semaphore;



char *my_read(){
	sem_wait(elements_to_read);
	if(writer_end == 1) {
		sem_post(elements_to_read);
		pthread_exit(0);
	}
	sem_wait(read_pointer_semaphore);
	char *res = array[read_pointer];
	array[read_pointer] = NULL;
	read_pointer++;
	read_pointer %= array_size;
	sem_post(read_pointer_semaphore);
	sem_post(elements_to_write);
	return res;
}

void my_write(char *line){
	sem_wait(elements_to_write);
	sem_wait(write_pointer_semaphore);
	array[write_pointer] = line;
	write_pointer++;
	write_pointer %= array_size;
	sem_post(write_pointer_semaphore);
	sem_post(elements_to_read);
}

void* writer(void *p){
	FILE *file = (FILE*)p;
	char *bufer = calloc(BUFER_SIZE, 1);
	while(fgets(bufer, BUFER_SIZE, file) != NULL){
		if(verbose){
			printf("Writer, tid %li, writing: |%s|\n", pthread_self(), bufer);
		}
		my_write(bufer);
		bufer = calloc(BUFER_SIZE, 1);
	}
}

void* reader(void *p){
	int L = *((int*)p);
	while(1){
		char *line = my_read();
		
		int line_len = strlen(line);
		if(line_len == L){
			printf("Length matched. Line: |%s|", line);
		}
		if(verbose){
			printf("Reader, tid %li, read: |%s|\n", pthread_self(), line);
		}
		free(line);
	}
}

void spawnProducers(int P, FILE *file){
	producers = malloc(sizeof(pthread_t) * P);
	for(int i = 0;i < P;i++){
		pthread_create(&producers[i], NULL, writer, (void*)file);
	}
}

void spawnConsumers(int K, int P){
	consumers = malloc(sizeof(pthread_t) * K);
	for(int i = 0;i < K;i++){
		pthread_create(&consumers[i], NULL, reader, (void*)&P);
	}
}

int main(int args, char *argv[]){
	read_pointer = write_pointer = elements_number = writer_end = 0;

	elements_to_read = malloc(sizeof(sem_t));
	elements_to_write = malloc(sizeof(sem_t));
	write_pointer_semaphore = malloc(sizeof(sem_t));
	read_pointer_semaphore = malloc(sizeof(sem_t));

	FILE* file = fopen(argv[1], "r");
	if(file == NULL){
		printf("Incorrect file\n");
		exit(1);
	}
	array_size = atoi(argv[2]);			//array size
	int P = atoi(argv[3]);				//producers number
	int K = atoi(argv[4]);				//consumers number
	verbose;						
	if(strcmp(argv[5], "-v") == 0){
		verbose = 1;
	} else {
		verbose = 0;
	}
	int nk = atoi(argv[6]);				//time to end
	int L = atoi(argv[7]);				//parameter of consumers

	array = malloc(sizeof(char*) * array_size);

	sem_init(elements_to_read, 0, 0);
	sem_init(elements_to_write, 0, array_size);
	sem_init(write_pointer_semaphore, 0, 1);
	sem_init(read_pointer_semaphore, 0, 1);

	spawnProducers(P, file);
	spawnConsumers(K, L);

	if(nk == 0) {
		signal(SIGINT, exit);
		for(int i = 0;i < P;i++) {
			pthread_join(producers[i], NULL);
		}
		writer_end = 1;
		sem_post(elements_to_read);
		for(int i = 0;i < K;i++) {
			pthread_join(consumers[i], NULL);
		}
	} else {
		signal(SIGALRM, exit);
		alarm(nk);
		pause();
	}
	fclose(file);
	return 0;
	return 0;
}