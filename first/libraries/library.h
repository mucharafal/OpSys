#ifndef FIRST_LIBRARY_H
#define FIRST_LIBRARY_H

//functions for dynamic allocated array
char** createArray(int size);
void freeArray(char **array);
int getSizeOfArray(char **array);

void addCharBlock(char **array, int sizeNewBlock);
void deleteCharBlock(char **pointerToBlock);
int getSizeOfBlock(char* block);
char* getBeginOfBlock(char* block);

int findElementSimilarSum(char **array, int index);
int sumElements(char* block);

//functions for static allocated array
void clearBlockStatic(char* array, int blockSize, int index);
void clearArrayStatic(char* array, int arraySize, int blockSize);

int findElementSimilarSumStatic(char* array, int arraySize, int blockSize, int index);
int sumElementsStatic(char* array, int blockSize, int index);

#endif