#include <stdio.h>
#include <limits.h>

int main(int args, char* argv[]){
	printf("Rodzic przed\n");
	
	int i = 0;
	int t = 1;
	char path[PATH_MAX];
	getcwd(path);
	strcat(path, "/");
	strcat(path, "IHello");
	printf("%s\n", path);
	if(args > 1)	i = atoi(argv[1]);
	for(int j = 0;j < i && t != 0;j++) {
		t = fork();
		if(t == 0) {
			
			strcat(path, "llll");
			printf("%s\n", path);
		}
	}
	wait();
	if(t != 0)	printf("%s\n", path);
	return 0;
}