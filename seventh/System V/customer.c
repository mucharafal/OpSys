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

int main(int args, char *argv[]){
    /*
    ./customer N S
    N - number of customers
    S - number of cut
    */
   if(args < 3) return 0;

	key_t key = ftok(".", 's');
	int semaphores = semget(key, 0, 0600);
    
    key = ftok("./barber.c", 's');
    int queue = msgget(key, 0600);

    int N = atoi(argv[1]);
    for(int i = 0; i <  N - 1; i++){
        if(fork() == 0){
            break;
        }
    }

    int S = atoi(argv[2]);
    for(int i = 0;i < S;){
        goIn(semaphores, CORRIDOR);

        //test śpiącego golibrody

        int val = semctl(semaphores, SLEEPING_BARBER, GETVAL);
        printf("Sleeping barber: %i\n", val);
        if(val == 0){
            incSemaphore(semaphores, SLEEPING_BARBER);

            //wyślij sygnał - przez kolejkę?
            struct msgbuf msgbuf;
            msgbuf.mtype = getpid();
            printf("Pid: %li\n", msgbuf.mtype);
            msgsnd(queue, &msgbuf, 0, 0);

            incSemaphore(semaphores, WAKE_BARBER);
            incSemaphore(semaphores, CORRIDOR);

            printTime("Customer, before cutting");
            incSemaphore(semaphores, BARBER_CHAIR);
            printTime("Customer, during cutting");
            goIn(semaphores, END_OF_CUTTING);
            printTime("Customer, end of cutting");
            i++;
            printf("Client cut\n");
        } else {
            //waiting room
            //sprawdź liczbę osób w poczekalni
            printf("In waiting room\n");
            int customersNumberInWR = semctl(semaphores, WAITING_CLIENTS, GETVAL);
            int limitCustomersNumberInWR = semctl(semaphores, LIMIT_WAITING_CLIENTS, GETVAL);
            if(customersNumberInWR < limitCustomersNumberInWR) {
                printTime("Customer, taking place in waiting room");
                incSemaphore(semaphores, WAITING_CLIENTS);
                struct msgbuf msgbuf;
                msgbuf.mtype = getpid();
                msgsnd(queue, &msgbuf, 0, 0);
                //zajmij odpowiednie krzesło
                
                printTime("After taking the place");
                int waitingCustomers = semctl(semaphores, WAITING_CLIENTS, GETVAL);
                printf("Waiting clients: %i\n", waitingCustomers);
                incSemaphore(semaphores, CORRIDOR);
                goIn(semaphores, customersNumberInWR + WAITING_ROOM_QUEUE);

                printTime("After taking place\n");
                
                //po zwolnieniu przechodź na następne
                //przetestować poprawność liczników
                for(int i = 0;i <= customersNumberInWR;i++){
                    
                    //błędy! odwołania poza semafory? i w ogóle to ogarnąć...
                    if(customersNumberInWR - i - 1 >= 0) {
                        incSemaphore(semaphores, customersNumberInWR - i + 1 + WAITING_ROOM_QUEUE);  //problem z rozbiciem kolejki w poczekalni i corridora na dwie grupy semaforów
                        goIn(semaphores, customersNumberInWR - i - 1 + WAITING_ROOM_QUEUE);
                    } else {
                        incSemaphore(semaphores, WAITING_ROOM_QUEUE + 1);
                    }
                }

                //zająć fotel
                printTime("Customer, before cutting");
                incSemaphore(semaphores, BARBER_CHAIR);
                printTime("Customer, during cutting");
                goIn(semaphores, END_OF_CUTTING);
                printTime("Customer, end of cutting");
                i++;

            } else {
                //go out
                incSemaphore(semaphores, CORRIDOR);
            }
        }
    }
    return 0;
}