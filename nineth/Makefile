monitor: monitor.o
	gcc monitor.o -o monitor -pthread

all: monitor semaphore

debug:
	gcc monitor.c -g -pthread

semaphore: semaphore.o
	gcc semaphore.o -o semaphore -pthread