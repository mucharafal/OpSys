#include <sys/types.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

int main(){
	time_t t;
	while(1) {
		sleep(1);
		time(&t);
		printf("%s", ctime(&t));
	}
	return 0;
}