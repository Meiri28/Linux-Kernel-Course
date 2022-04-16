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
#include <sys/shm.h>

#define SELF "./a.out"
#define MAX_DIRECTORY_NAME_LENGTH 255

void sighandler(int signum) {
	signal(SIGCONT, sighandler);
}

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
	waitpid(pid,NULL,0);
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
		sleep(0);
		execl(SELF, SELF, path, father, NULL);

		printf("failed %d\n", errno);
		exit(1);
	}
	// if father
	if (S_ISREG(path_stat.st_mode)) {
			pause();
	}
	else if (S_ISDIR(path_stat.st_mode)) {
		waitpid(pid, NULL, 0);
	}

	return pid;
}

int main(int argc, char * argv[]) {
	signal(SIGCONT, sighandler);
	key_t massage_queue_key = ftok("./message_queue", 'b');
	key_t average_word_count_shared_memory_key = ftok("./average_word_count_shared_memory", 'b');
	int massage_queue;
	int average_word_count_shared_memory;
	int* data;
	//root process
	if (argc == 2) {
		massage_queue = msgget(massage_queue_key, 0666 | IPC_CREAT);
		average_word_count_shared_memory = shmget(average_word_count_shared_memory_key, sizeof(int), 0666 | IPC_CREAT);
		data = shmat(average_word_count_shared_memory, (void*)0, 0);
		*data = -1;
	}
	else {
		massage_queue = msgget(massage_queue_key, 0666 | IPC_EXCL);
		average_word_count_shared_memory = shmget(average_word_count_shared_memory_key, sizeof(int), 0666 | IPC_EXCL);
	}

	struct stat file_stat;
	stat(argv[1], &file_stat);
	sleep(0);
	//printf("%d\n", average_word_count_shared_memory);
	//fflush(stdout);
	
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
		sleep(0);
		kill(atoi(argv[2]), SIGCONT);
		//printf("signaling %d %d with errno %d\n", atoi(argv[2]), , errno);
		sleep(0);
		//sleep(10);
		data = shmat(average_word_count_shared_memory, (void*)0, SHM_RDONLY);
		if (data == -1) {
			printf("data is -1\n");
			fflush(stdout);
			exit(1);
		}
		while (*data == -1) {
			//kill(atoi(argv[2]), SIGCONT);
			//printf("*data is -1\n");
		}
		printf("the diff foom the average for path %s is %d\n",argv[1], (*data) - massage.number_of_words);
		shmdt(data);
	}

	if (argc == 2) {
		struct word_count_massage massage;
		int count = 0;
		int sum = 0;
		long sq_sum = 0;
		while (msgrcv(massage_queue, &massage, sizeof(massage), 0, IPC_NOWAIT) > 0) {
			sum += massage.number_of_words;
			sq_sum += (massage.number_of_words * massage.number_of_words);
			count++;
		}
		int avg = sum / count;
		printf("the average is %d and the var is %d\n", avg, (sq_sum/count) - (avg*avg) );
		*data = avg;
		if (shmdt(data) == -1)
			printf("error in shmdt %d\n", errno);

		// Destroy message queue
		if (msgctl(massage_queue, IPC_RMID, NULL) == -1)
			printf("error destroying message queue");
		
		// Destroy shared mamory
		struct shmid_ds st;
		shmctl(average_word_count_shared_memory, IPC_STAT, &st);
		while (st.shm_nattch > 0) {
			sleep(0);
			shmctl(average_word_count_shared_memory, IPC_STAT, &st);
		}
		if (shmctl(average_word_count_shared_memory, IPC_RMID, NULL) == -1)
			printf("error destroying shared memory %d", errno);
	}

	fflush(stdout);
	return 0;
}

