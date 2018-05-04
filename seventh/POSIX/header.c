#include "header.h"

int addElement(fifo *queue, long element){
	if(queue->numberElements == queue->length)	return -1;

	int index = queue->firstElement + queue->numberElements;
	index %= queue->length;
	queue->pids[index] = element;
	queue->numberElements++;

	return queue->numberElements;
}

long takeElement(fifo *queue){
	if(queue->numberElements == 0)	return -1;

	long toReturn = queue->pids[queue->firstElement];

	queue->firstElement++;
	queue->firstElement %= queue->length;

	queue->numberElements--;

	return toReturn;
}	

int numberOfElements(fifo *queue){
	return queue->numberElements;
}

//from: https://www.daniweb.com/programming/software-development/threads/148080/itoa-function-or-similar-in-linux
char* Itoa(long int value, char* str, int radix) {
    static char dig[] =
        "0123456789"
        "abcdefghijklmnopqrstuvwxyz";
    long int n = 0, neg = 0;
    unsigned int v;
    char* p, *q;
    char c;
    if (radix == 10 && value < 0) {
        value = -value;
        neg = 1;
    }
    v = value;
    do {
        str[n++] = dig[v%radix];
        v /= radix;
    } while (v);
    if (neg)
        str[n++] = '-';
    str[n] = '\0';
    for (p = str, q = p + n - 1; p < q; ++p, --q) {
        c = *p, *p = *q, *q = c;
	}
    return str;
}



