//
// Created by rafal on 07.03.18.
//

#include <stdio.h>
#include <malloc.h>
#include <sys/times.h>
#include <sys/resource.h>
#include "libraries/library.h"

char staticArray[100000000]; 

void fillRand(char* array, int size){
    for(int i = 0;i < size;i++){
        array[i] = rand();
    }
}

int main(int argc, char* argv[]){

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
            
            switch(instruction){
            case 1:
                //initialization array
                clearArrayStatic(staticArray, arraySize, blockSize);
                break;
            case 2:
                argIndex++;
                findElementSimilarSumStatic(staticArray, arraySize, blockSize, atoi(argv[argIndex]));
                break;
            case 3:
                argIndex++;
                int addingBlocks = atoi(argv[argIndex]);
                argIndex++;
                indexToStart = atoi(argv[argIndex]);
                //clearing chosen blocks
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;
                    for(int j = 0 ; j < blockSize ; j++ ){
                        staticArray[i * blockSize + j] = 0;
                    }
                }
                //filling them
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;
                    fillRand(&staticArray[index], blockSize);
                }
                break;
            case 4:
                argIndex++;
                int blocksToDelete = atoi(argv[argIndex]);
                argIndex++;
                indexToStart = atoi(argv[argIndex]);
                for(int i = 0;i < blocksToDelete;i++){
                    int index = (indexToStart + i) % arraySize;
                    //clear and fill    //i know, its stupid
                    clearBlockStatic(staticArray, blockSize, index);
                    fillRand(staticArray, blockSize);
                }
                break;
            case 5:
                fillRand(staticArray, arraySize * blockSize);
                break;
            }
        } else {
            int indexToStart;
            //initialisation array
            dynamicArray = createArray(arraySize);
            for(int i = 0;i < arraySize;i++ ){
                addCharBlock(&dynamicArray[i], blockSize);
            }
            switch(instruction){
            case 1:
                break;
            case 2:
                argIndex++;
                findElementSimilarSum(dynamicArray, atoi(argv[argIndex]));
                break;
            case 3:
                argIndex++;
                int addingBlocks = atoi(argv[argIndex]);
                argIndex++;
                indexToStart = atoi(argv[argIndex]);

                //removing blocks
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;
                    deleteCharBlock(&dynamicArray[index]);
                }

                //creating new blocks
                for(int i = 0;i < addingBlocks; i++ ){
                    int index = (indexToStart + i) % arraySize;
                    addCharBlock(&dynamicArray[index], blockSize);
                    //fillRand(dynamicArray[index], blockSize);
                }
                break;

            case 4:
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

    printf("Real Time: %i, User Time %i, System Time %i\n",
        (endTime - startTime),
        (cpuEndTime.tms_utime - cpuStartTime.tms_utime),
        (cpuEndTime.tms_stime - cpuStartTime.tms_stime));
    return 0;
}
