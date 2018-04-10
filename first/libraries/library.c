#include "library.h"

#include <stdio.h>
#include <malloc.h>
#include <limits.h>

char** createArray(int size){
    //create new array and return a pointer to this
    //structure:
    //[pointerToSizeArray]:[Array]
    //pointer is set on array
    void **pointer = calloc(size + 1, sizeof(void*));
    int** sizeArray = (int**)pointer;
    *sizeArray = calloc(1, sizeof(int));
    *(*sizeArray) = size;
    pointer += 1;
    return (char**)pointer;
}

int getSizeOfArray(char** pointer){
    pointer--;                          //pointers have the same size
    int* size = (int*)pointer[0];
    return *size;
}

void freeArray(char** array){
    //free all blocks and array
    int size = getSizeOfArray(array);
    for(int i = 0;i < size;i++){
        if(array[i] != 0)
            deleteCharBlock(&array[i]);
    }
    array--;                            //see createArray()
    free(array[0]);
    free(array);
}

char* getBeginOfBlock(char* pointer){
    pointer -= sizeof(int) / sizeof(char);
    return pointer;
}

void deleteCharBlock(char** pointer){
    if(*pointer != 0) {
        char* beginPointer = getBeginOfBlock(*pointer);
        free(beginPointer);
    }
    *pointer = 0;
}

void addCharBlock(char** pointer, int sizeNewBlock) {
    //assume, that sizeof(char) <= sizeof(int)
    //[size of array: Int][array of chars]
    //pointer have address of first char in array
    if(*pointer != 0)   deleteCharBlock(pointer);
    *pointer = calloc(sizeNewBlock + sizeof(int) / sizeof(char), sizeof(char));
    int* size = (int*)*pointer;
    *pointer += sizeof(int) / sizeof(char);
    *size = sizeNewBlock;
}

int getSizeOfBlock(char* pointer){
    int* size = (int*)getBeginOfBlock(pointer);
    return *size;
}

int sumElements(char* block){
    if(block == 0)  return 0;
    int sum = 0;
    int blockSize = getSizeOfBlock(block);
    for(int i = 0;i < blockSize; i++){
        sum += block[i];
    }
    return sum;
}

int findElementSimilarSum(char** array, int index){
    //return index of the most similar element
    int main = sumElements(array[index]);
    int arraySize = getSizeOfArray(array);
    int theMostSimilar = 0;
    int theMostSimilarSum = INT_MAX;
    for(int i = 0;i < arraySize;i++){
        if(i != index){
            int sum = abs(sumElements(array[i]) - main);
            if(theMostSimilarSum > sum){
                theMostSimilar = i;
                theMostSimilarSum = sum;
            }
        }
    }

    return theMostSimilar;
}



void clearBlockStatic(char* array, int blockSize, int index){
    int endOfBlock = blockSize * (index + 1);
    for(int i = index * blockSize;i < endOfBlock;i++){
        array[i] = 0;
    }
}

void clearArrayStatic(char* array, int arraySize, int blockSize){
    int arrayTotalSize = arraySize * blockSize;
    for(int i = 0;i < arrayTotalSize;i++)    array[i] = 0;
}

int sumElementsStatic(char* array, int blockSize, int index){
    int sum = 0;
    int endOfBlock = blockSize * (index + 1);
    for(int i = index * blockSize;i < endOfBlock;i++){
        sum += array[i];
    }
    return sum;
}



int findElementSimilarSumStatic(char* array, int arraySize, int blockSize, int index){
    //return index of the most similar element
    int main = sumElementsStatic(array, blockSize, index);
    int theMostSimilar = 0;
    int theMostSimilarSum = INT_MAX;
    for(int i = 0;i < arraySize;i++){
        if(i != index){
            int sum = abs(sumElementsStatic(array, blockSize, index) - main);
            if(theMostSimilarSum > sum){
                theMostSimilar = i;
                theMostSimilarSum = sum;
            }
        }
    }

    return theMostSimilar;
}