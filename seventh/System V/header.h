#ifndef __HEADER_SYS_V

#define __HEADER_SYS_V

#define CORRIDOR 0
#define SLEEPING_BARBER 1
#define WAKE_BARBER 2
#define BARBER_CHAIR 3
#define END_OF_CUTTING 4
#define WAITING_CLIENTS 5
#define LIMIT_WAITING_CLIENTS 6
#define WAITING_ROOM_QUEUE 7

void goIn(int groupSemaphores, int numberSemaphore);
void incSemaphore(int groupSemaphores, int numberSemaphore);

union semun {
    int 			val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

struct msgbuf {
    long mtype;
    char mtext[1];
};

void printTime(char *name);

#endif