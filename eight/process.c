#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>

#define _DEFAULT_SOURCE 1
#define __USE_MISC
#include <string.h>
#undef __USE_MISC
#undef _DEFAULT_SOURCE
#include <pthread.h>
#include <math.h>
#include <time.h>
#define BUFER_SIZE 1024

int max(int a, int b){
	return (a > b)?a:b;
}
int min(int a, int b){
	return (a > b)?b:a;
}

struct arguments{
	int *image, x, y, filter_x, start_index, end_index;
	double *filter, *result;
};

void load_imageAAA(FILE *imageFile, int **t, int *x, int *y){
	
	char *bufer;
	bufer = malloc(1024);
	int buferSize = 1024;
	int wasSize = 0, wasMax = 0;
	int i = 0;
	int j = 0;

	int read;

	
	while((bufer = fgets(bufer, buferSize, imageFile)) != NULL){
		int skip = 0;
		if(bufer[0] == '#')	skip = 1;
		if(bufer[0] == 'P' && bufer[1] == '2') skip = 1;
		if(!wasSize && !skip){
			char *firstNum = strsep(&bufer, " ");
			char *secondNum = strsep(&bufer, " ");
			bufer = firstNum;

			*x = atoi(firstNum);
			*y = atoi(secondNum);

			t[0] = malloc(*x * *y * sizeof(int));		

			wasSize = 1;
			skip = 1;
		}
		if(wasSize && !wasMax && !skip) {
			wasMax = 1;
			skip = 1;
		}

		if(wasSize && wasMax && !skip) {
			char *tempBufer = bufer;
			char *number;
			
			while((number = strsep(&bufer, " \t\n")) != NULL){
				if(strcmp(number, "") != 0) {
					t[0][i * *x + j] = atoi(number);
					j++;
					if(j == *x) {
						i++;
						j = 0;
					}
				}
			}
			bufer = tempBufer;
		}		
	}
}

void load_filterAAA(FILE *filterFile, double **t, int *x){
	char *bufer;
	bufer = malloc(1024);
	int buferSize = 1024;
	int wasSize = 0;
	int i = 0;

	int read;

	
	while((bufer = fgets(bufer, buferSize, filterFile)) != NULL){
		int skip = 0;
		if(!wasSize){
			char *firstNum = strsep(&bufer, " \n");
			bufer = firstNum;

			*x = atoi(bufer);

			t[0] = calloc(*x * *x, sizeof(double));

			wasSize = 1;
			skip = 1;
		}
		if(wasSize && !skip) {
			char *number;
			
			char *tempBufer = bufer;
			while((number = strsep(&bufer, " ")) != NULL){
				t[0][i] = atof(number);
				i++;
			}
			bufer = tempBufer;
		}
	}
}

void* process(void *p){	
	struct arguments *args = (struct arguments*)p;
	int *image = args->image;
	int c = args->filter_x;
	int half_c_normalised = ceil(c / 2);
	for(int y = args->start_index; y < args->end_index; y++){
		for(int x = 0, index = y * args->x; x < args->x;x++, index++){
			double s = 0;
			for(int i = 0;i < c;i++){
				for(int j = 0;j < c;j++){
					s += image[max(0, min(x - half_c_normalised + i, args->x)) + max(0, min(y - half_c_normalised + i, args->y)) * args->x] * args->filter[i + j * c];
				}
			}
			args->result[x + args->x * y] = round(s);
		}
	}
	pthread_exit(0);
}

int main(int args, char *argv[]){
	/*
	argv[1] - path to image
	argv[2] - path to filter
	argv[3] - number of threads
	*/
	
	int threadsNumber = atoi(argv[3]);
	printf("Threads: %i\n", threadsNumber);
	FILE *imageFile = fopen(argv[1], "r");
	if(imageFile == NULL) printf("NULL\n");
	int **imagePointer = malloc(sizeof(int*)), *x = malloc(sizeof(int)), *y = malloc(sizeof(int));
	load_imageAAA(imageFile, imagePointer, x, y);
	fclose(imageFile);
	int m = *y * *x;
	int *image = imagePointer[0];

	double **filterPointer = malloc(sizeof(double));
	int *filter_x = malloc(sizeof(int));
	FILE *filterFile = fopen(argv[2], "r");
	load_filterAAA(filterFile, filterPointer, filter_x);
	double *filter = filterPointer[0];
	fclose(filterFile);

	m = *filter_x* *filter_x;

	double *result = malloc(*x * *y * sizeof(double));

	struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	int startIndex = 0;
	int endIndex = 0;
	pthread_t threads[threadsNumber];
	for(int startIndex = 0, i = 0, endIndex = (int)round(*y * ((float)(i + 1) / threadsNumber));
			i < threadsNumber;
			i++, startIndex = endIndex, endIndex = (int)round(*y * ((float)(i + 1) / threadsNumber))){
		struct arguments *args = malloc(sizeof(struct arguments));
		args->end_index = endIndex;
		args->start_index = startIndex;
		args->x = *x;
		args->y = *y;
		args->result = result;
		args->image = image;
		args->filter_x = *filter_x;
		args->filter = filter;
		pthread_create(&threads[i], NULL, &process, (void*)args);
	}

	for (int i = 0; i < threadsNumber; i++)
    	pthread_join(threads[i], NULL);
//czekanie na wszystkie
	struct timespec end;
	clock_gettime(CLOCK_MONOTONIC, &end);
	long long int totalTimeNs = (long long int)(end.tv_sec - start.tv_sec) * 1000000000 + (long long int)(end.tv_nsec - start.tv_nsec);
	printf("Time: %li s, %li \n", totalTimeNs / 1000000000, totalTimeNs % 1000000000);

	FILE *save = fopen(argv[4], "w");
	char buf[1024] = {0};

	fprintf(save, "P2\n");
	fprintf(save, "%i %i\n", *x, *y);
	fprintf(save, "255\n");

	for(int i = 0 ; i < *x * *y;i++){
		fprintf(save, "%i ", (int)result[i]);
		if(i % *x == (*x - 1)){
			fprintf(save, "\n");
		}
	}

	fclose(save);

	return 0;
}