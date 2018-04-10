#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <string.h>

void generateRecord(unsigned char *record, int size){
	for(int i = 0; i < size ; i++){
		record[i] = rand()%256;
	}
}
int main(int args, char* argv[]){
	clock_t startTime;
    struct tms cpuStartTime;
    //czasy dodac
    startTime = times(&cpuStartTime);
	if(args == 6){
		//generate or sort
		//argv:
		//1 - generate or sort
		//2 - fileName
		//3 - arraySize
		//4 - recordSize
		//5 - sys or lib
		int arraySize = atoi(argv[3]);
		int recordSize = atoi(argv[4]);
		unsigned char* record = calloc(recordSize, sizeof(char));
		if(strcmp(argv[1], "generate") == 0){
			if(strcmp(argv[5], "sys") == 0){
				int handle = open(argv[2], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				for(int i = 0;i < arraySize;i++){
					generateRecord(record, recordSize);
					write(handle, record, recordSize);
				}
				close(handle);
			} else {
				FILE *file = fopen(argv[2], "w");
				for(int i = 0;i < arraySize;i++){
					generateRecord(record, recordSize);
					fwrite(record, sizeof(char), recordSize, file);
				}
				fclose(file);
			}
		} else {
			//sort
			unsigned char *recordToInsert = malloc(sizeof(char) * recordSize);
			if(strcmp(argv[5], "sys") == 0){
				int currentPosition = 0;
				int handle = open(argv[2], O_RDWR);
				int memoryRecordSize = sizeof(char) * recordSize;
				for(int i = 1;i < arraySize;i++){

					int wasSwap = 0;

					lseek(handle, memoryRecordSize * (i - 1), SEEK_SET);
					read(handle, record, recordSize);
					read(handle, recordToInsert, recordSize);
					while(record[0] > recordToInsert[0]){
						wasSwap = 1;
						lseek(handle, -memoryRecordSize, SEEK_CUR);
						write(handle, record, recordSize);

						if(lseek(handle, -memoryRecordSize * 3, SEEK_CUR) >= 0){
							read(handle, record, recordSize);
							lseek(handle, memoryRecordSize, SEEK_CUR);
						} else {
							lseek(handle, memoryRecordSize, SEEK_SET);
							break;
						}
					}
					if(wasSwap){
						lseek(handle, -memoryRecordSize, SEEK_CUR);
						write(handle, recordToInsert, recordSize);
					}
				}
				close(handle);
			} else {
				FILE *file = fopen(argv[2], "r+");
				int memoryRecordSize = sizeof(char) * recordSize;

				for(int i = 1;i < arraySize;i++){
					int wasSwap = 0;
					fseek(file, (i - 1) * memoryRecordSize, 0);
					fread(record, sizeof(char), recordSize, file);
					fread(recordToInsert, sizeof(char), recordSize, file);
					while(record[0] > recordToInsert[0]){
						wasSwap = 1;
						fseek(file, -memoryRecordSize, 1);
						fwrite(record, sizeof(char), recordSize, file);
						if(fseek(file, -memoryRecordSize * 3, 1) == 0){
							fread(record, sizeof(char), recordSize, file);
							fseek(file, memoryRecordSize, 1);
						} else {
							fseek(file, memoryRecordSize, SEEK_SET);
							break;
						}
					}
					if(wasSwap){
						fseek(file, -memoryRecordSize, 1);
						fwrite(recordToInsert, sizeof(char), recordSize, file);
					}
				}
				fclose(file);
			}
		}

	} else {
		if(args == 7){
			//1 - copy
			//2 - source file name
			//3 - destiny file name
			//4 - array size
			//5 - record size
			//6 - sys or lib
			int arraySize = atoi(argv[4]);
			int recordSize = atoi(argv[5]);
			unsigned char* record = calloc(recordSize, sizeof(char));

			if(strcmp(argv[6], "sys") == 0){
				int source = open(argv[2], O_RDONLY);
				int destiny = open(argv[3], O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
				for(int i = 0;i < arraySize;i++){
					read(source, record, recordSize);
					write(destiny, record, recordSize);
				}
				close(source);
				close(destiny);
			} else {
				FILE *source = fopen(argv[2], "r");
				FILE *destiny = fopen(argv[3], "w");
				for(int i = 0;i < arraySize;i++){
					fread(record, sizeof(char), recordSize, source);
					fwrite(record, sizeof(char), recordSize, destiny);
				}
				fclose(source);
				fclose(destiny);
			}
		}
	}
	clock_t endTime;
    struct tms cpuEndTime;
    endTime = times(&cpuEndTime);

    printf("Real Time: %li, User Time %li, System Time %li\n",
        (endTime - startTime),
        (cpuEndTime.tms_utime - cpuStartTime.tms_utime),
        (cpuEndTime.tms_stime - cpuStartTime.tms_stime));

	return 0;
}
