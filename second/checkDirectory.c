#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/times.h>
#include <time.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>
#include <linux/limits.h>

int sign;
time_t argumentTime;

void printFileInformation(char* pathWithName, struct stat fileStat){

	printf("Path to file: %s\n", pathWithName);
	printf("File size in bytes: %i\n", (int)fileStat.st_size);

	char filePermissions[16];
	strcpy(filePermissions, (fileStat.st_mode & S_IRUSR) ? "r" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IWUSR) ? "w" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IXUSR) ? "x" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IRGRP) ? "r" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IWGRP) ? "w" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IXGRP) ? "x" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IROTH) ? "r" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IWOTH) ? "w" : "-");
	strcat(filePermissions, (fileStat.st_mode & S_IXOTH) ? "x" : "-");

	printf("Permissions: %s\n", filePermissions);
	char lastModificationTime[80];
	struct tm *info;
	info = localtime(&fileStat.st_mtime);
	strftime(lastModificationTime, 80, "%a, %d.%m.%Y %H:%M:%S", info);

	printf("Last modification: %s\n", lastModificationTime );

}

void searchDirectory(char* absolutePath){


	DIR *handle = opendir(absolutePath);

	struct dirent* file;
	struct stat fileStat;

	while((file = readdir(handle))){
		char pathWithName[PATH_MAX];
		strcpy(pathWithName, absolutePath);
		strcat(pathWithName, "/");
		strcat(pathWithName, file->d_name);

		lstat(pathWithName, &fileStat);
		if(S_ISLNK(fileStat.st_mode) == 0) {
			if(S_ISREG(fileStat.st_mode)) {
				double timeDifference = difftime(fileStat.st_mtime, argumentTime);
				if(timeDifference * sign > 0 || (timeDifference == 0 && sign == 0)){
					printFileInformation(pathWithName, fileStat);
				}
			} else {
				if(S_ISDIR(fileStat.st_mode) &&
					strcmp(file->d_name, ".") != 0 &&
					strcmp(file->d_name, "..") != 0) {
					searchDirectory(pathWithName);
				}
			}
		}
	}
	closedir(handle);
}

int nftwShow(char *path, struct stat *fileStat, int fileType, void *foo3) {
	if(fileType == FTW_F && fileType != FTW_SL) {
		double timeDifference = difftime(fileStat->st_mtime, argumentTime);
		if((timeDifference * sign) > 0 || (timeDifference == 0 && sign == 0)){
			printFileInformation(path, *fileStat);
		}
	}
	return 0;
}

int main(int args, char* argv[]){
	if(args > 4){
		//parsing command line arguments
		//getting absolute path to folder to start
		char absolutePath[PATH_MAX];
		realpath(argv[1], absolutePath);
		//time to compare
		struct tm tm;
		strptime(argv[2], "%d.%m.%Y,%H:%M:%S", &tm);
		argumentTime = mktime(&tm);
		//sign
		if(strcmp("<", argv[3]) == 0)	sign = 1;		//later date than argumentTime
		if(strcmp("=", argv[3]) == 0)	sign = 0;		//date is equal argumentTime
		if(strcmp(">", argv[3]) == 0)	sign = -1;		//earlier date than argumentTime
		if(strcmp(argv[4], "nftw") != 0) {
			searchDirectory(absolutePath);
		} else {
			nftw(absolutePath, &nftwShow, 10, FTW_PHYS);
		}
	}
	return 0;
}
