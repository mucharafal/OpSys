#define _POSIX_C_SOURCE 20000000

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <string.h>
#include <mqueue.h>
#include <errno.h>
#include <sys/sem.h>
//queue
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "header.h"


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

void goIn(int semaphores, int semaphore){
    struct sembuf sops[1];
    sops[0].sem_num = semaphore;
    sops[0].sem_op = -1;
    sops[0].sem_flg = 0;
    semop(semaphores, sops, 1);
}

void incSemaphore(int semaphores, int semaphore){
    struct sembuf sops[1];
    sops[0].sem_num = semaphore;
    sops[0].sem_op = 1;
    sops[0].sem_flg = 0;
    semop(semaphores, sops, 1);
}

int main(int args, char *argv[]){
    /*
    ./customer N S
    N - number of customers
    S - number of cut
    */
   if(args < 3) return 0;

	key_t key = ftok(".", 's');
	int semaphores = semget(key, 0, 0600);
    
    key = ftok("./communicateWithBarberQueue", 's');
    int queue = msgget(key, 0600);

    int N = atoi(argv[1]);
    for(int i = 0; i <  N - 1; i++){
        if(fork() == 0){
            break;
        }
    }

    int S = atoi(argv[2]);
    for(int i = 0;i < S;i++){
        printf("Inside...\n");

        goIn(semaphores, CORRIDOR);

        //test śpiącego golibrody

        int val = semctl(semaphores, SLEEPING_BARBER, GETVAL);
        printf("Val: %i\n", val);
        if(val == 0){
            incSemaphore(semaphores, SLEEPING_BARBER);

            //wyślij sygnał - przez kolejkę?
            struct msgbuf msgbuf;
            msgbuf.mtype = getpid();
            msgsnd(queue, &msgbuf, 0, 0);

            incSemaphore(semaphores, WAKE_BARBER);
            incSemaphore(semaphores, CORRIDOR);

            incSemaphore(semaphores, BARBER_CHAIR);
            goIn(semaphores, END_OF_CUTTING);
        } else {
            //waiting room
            //sprawdź liczbę osób w poczekalni
            int customersNumberInWR = semctl(semaphores, WAITING_CLIENTS, GETVAL);
            int limitCustomersNumberInWR = semctl(semaphores, LIMIT_WAITING_CLIENTS, GETVAL);
            if(customersNumberInWR < limitCustomersNumberInWR) {
                incSemaphore(semaphores, WAITING_CLIENTS);
                struct msgbuf msgbuf;
                msgbuf.mtype = getpid();
                msgsnd(queue, &msgbuf, 0, 0);
                //zajmij odpowiednie krzesło
                //po zwolnieniu przechodź na następne
                for(int i = 0;i < customersNumberInWR;i++){
                    struct sembuf sops[2];
                    sops[0].sem_num = customersNumberInWR - i + WAITING_ROOM_QUEUE;  //problem z rozbiciem kolejki w poczekalni i corridora na dwie grupy semaforów
                    //pożądana operacja atomowa: zwolnij poczekalnię, zablokuj się na odpowiednim krześle
                    //nast po obudzeniu blokuj korytarz, zwolnij za tobą, zablokuj się na następnym. 
                    sops[0].sem_op = -1;
                    sops[0].sem_flg = 0;
                    sops[1].sem_num = WAITING_ROOM_QUEUE - i - 1 + WAITING_ROOM_QUEUE;
                    sops[1].sem_op = 1;
                    sops[1].sem_flg = 0;
                    semop(semaphores, sops, 2);
                }

            } else {
                //go out
                incSemaphore(semaphores, CORRIDOR);
            }
        }
    }
    return 0;
}