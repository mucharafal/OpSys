#ifndef __HEADER_POSIX

#define __HEADER_POSIX

#define CORRIDOR 0
#define SLEEPING_BARBER 1
#define WAKE_BARBER 2
#define BARBER_CHAIR 3
#define END_OF_CUTTING 4

typedef struct{
	int length;
	int firstElement;
	int numberElements;
	long pids[];
}fifo;

int addElement(fifo *queue, long element);
long takeElement(fifo *queue);
char *Itoa(long value, char *str, int radix);

#endif