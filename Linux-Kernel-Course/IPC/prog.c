#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define SELF "IPC.out"
#define MAX_DIRECTORY_NAME_LENGTH 255

int create_child(char* argv)
{
	int pid = fork();
	//check if child
	if (pid == 0) {
		execl(SELF, SELF, argv);
		printf("failed %d\n", errno);
	}

	return pid;
}

int main(int argc, char * argv[]) {
	//printf("%s %s\n", argv[0], argv[1]);
	struct stat file_stat;
	stat(argv[1], &file_stat);
	
	// IS DIR
	if (S_ISDIR(file_stat.st_mode)) {
		struct DIR* current_dir = opendir(argv[1]);
		struct dirent* dp;
		// read ..
		dp = readdir(current_dir);
		// read .
		dp = readdir(current_dir);
		// read first child dir
		dp = readdir(current_dir);
		char* full_path = calloc(strlen(argv[1]) + MAX_DIRECTORY_NAME_LENGTH, sizeof(char));
		while (dp != NULL) {
			strcpy(full_path, argv[1]);
			strcat(full_path, dp->d_name);
			strcat(full_path, "/");
			create_child(full_path);
			
			dp = readdir(current_dir);
		}

		free(full_path);
		close(current_dir);
	}
	// IS REGULAR FILE
	else if (S_ISREG(file_stat.st_mode)) {
		//printf("%s\n", argv[1]);
	}
		
	//switch (file_stat.st_mode)
	//{
	//case S_IFMT:
	//	printf("file");
	//	break;
	//case S_IFDIR:
	//	printf("directory");
	//	break;
	//default:
	//	printf("UNKNOWN");
	//	break;
	//}

	return 0;
}

