#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/ipc.h>

#define SELF "./a.out"
#define MAX_DIRECTORY_NAME_LENGTH 255

struct word_count_massage
{
	long mtype;
	int number_of_words;
};

int read_int_from_fd(int fd) {
	int result = 0;
	char current;
	while (read(fd, &current, 1) && current >= '0' && current <= '9') {
		result *= 10;
		result += current - '0';
	}

	return result;
}

int run_wc_and_get_result(char* path) {
	int pfds[2];
	if (pipe(pfds) != 0)
		printf("pipe error\n");
	int pid = fork();
	if (pid == 0) {
		close(1);
		dup(pfds[1]);
		execl("/usr/bin/wc", "/usr/bin/wc", "-w", path, NULL);
		printf("failed to execl %d\n", errno);
		fflush(stdout);
	}
	wait(NULL);
	int result = read_int_from_fd(pfds[0]);
	close(pfds[0]);
	close(pfds[1]);
	
	return result;
}

int create_child(char* path)
{
	struct stat path_stat;
	stat(path, &path_stat);
	char father[20];
	sprintf(father, "%d", getpid());
	int pid = fork();
	//check if child
	if (pid == 0) {
		execl(SELF, SELF, path, father);

		printf("failed %d\n", errno);
		exit(1);
	}
	wait(NULL);
	// if father
	/*if (S_ISREG(path_stat.st_mode)) {
		char current = '0';
		while (current != '1') {
			sleep(0);
			read(pfds[0], &current, 1);
		}
	}
	else if (S_ISDIR(path_stat.st_mode)) {
		
	}*/

	return pid;
}

int main(int argc, char * argv[]) {
	key_t massage_queue_key = ftok("./message_queue", 'b');
	int massage_queue;
	//root process
	if (argc == 2) {
		massage_queue = msgget(massage_queue_key, 0666 | IPC_CREAT);
	}
	else {
		massage_queue = msgget(massage_queue_key, 0666 | IPC_EXCL);
	}

	struct stat file_stat;
	stat(argv[1], &file_stat);
	sleep(0);
	
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
			strcat(full_path, "/");
			strcat(full_path, dp->d_name);
			create_child(full_path);
			dp = readdir(current_dir);
		}

		free(full_path);
		close(current_dir);
	}
	// IS REGULAR FILE
	else if (S_ISREG(file_stat.st_mode)) {
		struct word_count_massage massage = { 1, run_wc_and_get_result(argv[1]) };
		int return_code = msgsnd(massage_queue, &massage, sizeof(massage), 0);
		// tell father that you sent the first process the data he need and now you wait for final result from him.
		signal(SIGCONT, atoi(argv[2]));
	}

	if (argc == 2) {
		struct word_count_massage massage;
		int count = 0;
		while (msgrcv(massage_queue, &massage, sizeof(massage), 0, IPC_NOWAIT) > 0) {
			printf("file with %d words\n", massage.number_of_words);
			count++;
		}
		printf("got %d massages errno %d\n", count, errno);
	}

	fflush(stdout);
	return 0;
}

