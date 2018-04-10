//
// Created by rafal on 07.03.18.
//

#include <stdio.h>
#include <malloc.h>
#include <sys/times.h>
#include <sys/resource.h>
#include <dlfcn.h>

char staticArray[100000000]; 

void fillRand(char* array, int size){
    for(int i = 0;i < size;i++){
        array[i] = rand();
    }
}

int main(int argc, char* argv[]){
	void *handle;

   	handle = dlopen("libDynamicLibrary.so.1.0.0", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "%s\n", dlerror());
		return 1;
    }

   	dlerror();    /* Clear any existing error */
    if(argc < 5){
        printf("Incorrect number of arguments");
        return 0;
    }

    clock_t startTime;
    struct tms cpuStartTime;
    //czasy dodac
    startTime = times(&cpuStartTime);

    int arraySize = atoi(argv[1]);
    int blockSize = atoi(argv[2]);
    char** dynamicArray;
    for(int argIndex = 4;argIndex < argc;argIndex++){
        int indexToStart;
        int instruction = atoi(argv[argIndex]);
        if(*argv[3] == 'S'){
			void (*clearArrayStatic)(char* a, int b, int c);
			clearArrayStatic = dlsym(handle, "clearArrayStatic"); 
            switch(instruction){
            case 1:
				//initialization array - 1 argument
				(*clearArrayStatic)(staticArray, arraySize, blockSize);
                break;
            case 2:
                //find element - 2 argument
                argIndex++;
				int (*findElementSimilarSumStatic)() = (int(*)())dlsym(handle, "findElementSimilarSumStatic"); 
                (*findElementSimilarSumStatic)(staticArray, arraySize, blockSize, atoi(argv[argIndex]));
                break;
            case 3:
                //clear n -block and later fill them    - 3 args
                argIndex++;
                int addingBlocks = atoi(argv[argIndex]);
                argIndex++;
                indexToStart = atoi(argv[argIndex]);
                //clearing chosen blocks
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;
					void (*clearBlockStatic)() = dlsym(handle, "clearBlockStatic"); 
                    (*clearBlockStatic)(staticArray, blockSize, index);
                }
                //filling them
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;
                    fillRand(&staticArray[index], blockSize);
                }
                break;
            case 4:
                //clear and fill n-times    - 3 args
                argIndex++;
                int blocksToDelete = atoi(argv[argIndex]);
                argIndex++;
                indexToStart = atoi(argv[argIndex]);
                for(int i = 0;i < blocksToDelete;i++){
                    int index = (indexToStart + i) % arraySize;
                    //clear and fill    //i know, its stupid
                    void (*clearBlockStatic)() = dlsym(handle, "clearBlockStatic"); 
                    (*clearBlockStatic)(staticArray, blockSize, index);
                    fillRand(staticArray, blockSize);
                }
                break;
            case 5:
                //fill array - 1 arg
                fillRand(staticArray, arraySize * blockSize);
                break;
            }
        } else {
            int indexToStart;
			void (*addCharBlock)() = dlsym(handle, "addCharBlock"); 
			void (*deleteCharBlock)() = dlsym(handle, "deleteCharBlock"); 
			char** (*createArray)() = (char**(*)())dlsym(handle, "createArray"); 
            dynamicArray = (*createArray)(arraySize);
            for(int i = 0;i < arraySize;i++ ){
                (*addCharBlock)(&dynamicArray[i], blockSize);
            }
            switch(instruction){
            case 1:
                //initialisation array  - 1 arg
                
                break;
            case 2:
                //find element  - 2 args
                argIndex++;
				int (*findElementSimilarSum)() = (int(*)())dlsym(handle, "findElementSimilarSum"); 
                (*findElementSimilarSum)(dynamicArray, atoi(argv[argIndex]));
                break;
            case 3:
                //remove n blocks, next add n blocks    3 args- type of instruction, n, place to start
                //n < arraySize
                argIndex++;
                int addingBlocks = atoi(argv[argIndex]);
                argIndex++;
                indexToStart = atoi(argv[argIndex]);

                //removing blocks
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;

                    (*deleteCharBlock)(&dynamicArray[index]);
                }

                //creating new blocks
				
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;
                    (*addCharBlock)(&dynamicArray[index], blockSize);
                    //fillRand(dynamicArray[index], blockSize);
                }
                break;

            case 4:
                //remove and alocate n times -  3 args - type of instruction, n, place to start
                argIndex++;
                int blocksToDelete = atoi(argv[argIndex]);
                argIndex++;
                indexToStart = atoi(argv[argIndex]);
                for(int i = 0;i < blocksToDelete;i++){
                    int index = (indexToStart + i) % arraySize;
                    deleteCharBlock(&dynamicArray[index]);
                    addCharBlock(&dynamicArray[index], blockSize);
                    //fillRand(dynamicArray[index], blockSize);
                }
                break;
            case 5:
                for(int i = 0; i < arraySize;i++){
                    fillRand(dynamicArray[i], blockSize);
                }
            }
        }
    }
    clock_t endTime;
    struct tms cpuEndTime;
    endTime = times(&cpuEndTime);
	dlclose(handle);
    printf("Real Time: %i, User Time %i, System Time %i\n",
        (endTime - startTime),
        (cpuEndTime.tms_utime - cpuStartTime.tms_utime),
        (cpuEndTime.tms_stime - cpuStartTime.tms_stime));
    return 0;
}
