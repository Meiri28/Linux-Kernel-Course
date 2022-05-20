#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/poll.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MAX_HTTP_HEADER_SIZE 4096

struct addrinfo {
	int              ai_flags;
	int              ai_family;
	int              ai_socktype;
	int              ai_protocol;
	socklen_t        ai_addrlen;
	struct sockaddr* ai_addr;
	char* ai_canonname;
	struct addrinfo* ai_next;
};

enum request_status
{
	wait_for_send,
	wait_for_rcv_header,
	wait_for_rcv_body,
	finish,
};

struct MyStruct
{
	char* request;
	char* file_path;
	int dest_fd;
	int fd;
	enum request_status status;
	char headr[MAX_HTTP_HEADER_SIZE];
	int byte_upload;
	int byte_download_to_header;
	int bytes_download_to_body;
};

int running;
int number_of_requests;
struct MyStruct* requests;

char* create_get_request(char* domain, char* file)
{
	char* request = malloc(sizeof(char) * (100 + strlen(domain) + strlen(file)));
	strcpy(request, "GET ");
	strcat(request, file);
	strcat(request, " HTTP/1.0\r\nHOST: ");
	strcat(request, domain);
	strcat(request, "\r\n\r\n\r\n");

	return request;
}

int connect_http_with_domain(char* domain) 
{
	struct addrinfo hints;
	struct addrinfo *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(domain, "80", &hints, &res) != 0) {
			printf("error in getaddrinfo errno %d\n", errno);
			exit(1);
	}
	int sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	connect(sockfd, res->ai_addr, res->ai_addrlen);

	freeaddrinfo(res);
	
	return sockfd;
}

void stop(int num) 
{
	if (num >= number_of_requests)
		return;
	close(requests[num].fd);
	close(requests[num].dest_fd);
	free(requests[num].file_path);
	free(requests[num].request);
	while (num < number_of_requests - 1)
	{
		requests[num] = requests[num + 1];
		num++;
	}
	number_of_requests--;
}

void show()
{
	for (int i = 0; i < number_of_requests; i++)
	{
		struct MyStruct current = requests[i];
		printf("%d. http://%s %d\n", i, current.request, current.bytes_download_to_body);
	}
}

char* get_domain_from_requests(char* request)
{
	if (strncmp(request, "http://", sizeof(char) * strlen("http://")) != 0) {
		printf("the request must start with http://\n");
		exit(1);
	}
	char* url = request + strlen("http://");
	char* file = strstr(url, "/");
	char* result = malloc(sizeof(char) * (int)(file - url + 1));
	strncpy(result, url, file - url);
	result[url - file] = '\0';

	return result;
}

char* get_path_from_request(char* request) 
{
	char* url = request + strlen("http://");
	char* file = strstr(url, "/");
	char* result = malloc(sizeof(char) * (strlen(file) + 1));
	strcpy(result, file);

	return result;
}

int get_fd_from_request(char* request) 
{
	char* domain = get_domain_from_requests(request);
	int f = connect_http_with_domain(domain);
	free(domain);
	fcntl(f, F_SETFL, O_NONBLOCK);

	return f;
}

void get_command() {
	char* line = NULL;
	int size = 0;
	getline(&line, &size, stdin);
	if (strncmp(line, "show", sizeof(char) * strlen("show")) == 0) {
		show();
	} else if (strncmp(line, "stop", sizeof(char) * strlen("stop")) == 0) {
		stop(atoi(line + strlen("stop") + 1));
	} else if (strncmp(line, "start", sizeof(char) * strlen("start")) == 0) {
		requests = realloc(requests, sizeof(struct MyStruct) * (number_of_requests + 1));
		number_of_requests++;
		char* url = strstr(line, " ") + 1;
		char* endurl = strstr(url, " ");
		requests[number_of_requests - 1].request = malloc(sizeof(char) * (int)(endurl - url + 1));
		strncpy(requests[number_of_requests - 1].request, url, endurl - url);
		requests[number_of_requests - 1].request[endurl - url] = '\0';
		requests[number_of_requests - 1].file_path = malloc(sizeof(char) * strlen(endurl + 1));
		strcpy(requests[number_of_requests - 1].file_path, endurl + 1);
		requests[number_of_requests - 1].file_path[strlen(requests[number_of_requests - 1].file_path) - 1] = '\0';
		requests[number_of_requests - 1].dest_fd = open(requests[number_of_requests - 1].file_path, O_CREAT| O_WRONLY, S_IRWXU);		
		requests[number_of_requests - 1].bytes_download_to_body = 0;
		requests[number_of_requests - 1].byte_download_to_header = 0;
		requests[number_of_requests - 1].byte_upload = 0;
		requests[number_of_requests - 1].status = wait_for_send;
		requests[number_of_requests - 1].fd = get_fd_from_request(requests[number_of_requests - 1].request);
	} else if (strncmp(line, "leave", sizeof(char) * strlen("leave")) == 0) {
		running = 0;
	}

	free(line);
}

void sent_header(int num)
{
	char* domain = get_domain_from_requests(requests[num].request);
	char* path = get_path_from_request(requests[num].request);
	char* request = create_get_request(domain, path);
	free(domain);
	free(path);
	
	requests[num].byte_upload += send(requests[num].fd, request + requests[num].byte_upload, strlen(request + requests[num].byte_upload), 0);
	if (requests[num].byte_upload == strlen(request))
	{
		requests[num].status = wait_for_rcv_header;
	}
	free(request);
}

int body_size(char* header) 
{
	char* a = strstr(header, "Content-Length: ");

	return atoi(a + (sizeof(char) * strlen("Content-Length: ")));;
}

void receve_http(int num) 
{
	if (requests[num].status == wait_for_rcv_header) {
		requests[num].byte_download_to_header += recv(requests[num].fd, &requests[num].headr + requests[num].byte_download_to_header, 4096 - requests[num].byte_download_to_header, 0);
		char* end_of_header = strstr(requests[num].headr, "\r\n\r\n");
		if (end_of_header != NULL)
		{
			*end_of_header = '\0';
			requests[num].status = wait_for_rcv_body;
			char* start_of_body = end_of_header + sizeof(char) * strlen("\r\n\r\n");
			int a = write(requests[num].dest_fd, start_of_body, strlen(start_of_body));
			requests[num].bytes_download_to_body += strlen(start_of_body);
		}
	} else {
		char body[500];
		int byte_downloaded = recv(requests[num].fd, body, 500, 0);
		requests[num].bytes_download_to_body += byte_downloaded;
		write(requests[num].dest_fd, body, byte_downloaded);
	}
	if (requests[num].status == wait_for_rcv_body && body_size(requests[num].headr) <= requests[num].bytes_download_to_body){
		requests[num].status = finish;
	}
}

int main() {
	running = 1;
	number_of_requests = 0;
	requests = NULL;
	while (running) {
		struct pollfd* poll_request = malloc(sizeof(struct pollfd) * (number_of_requests + 1));
		for (int i = 0; i < number_of_requests; i++) {
			poll_request[i].fd = requests[i].fd;
			if (requests[i].status == wait_for_send)
				poll_request[i].events = POLLOUT;
			else
				poll_request[i].events = POLLIN;
		}
		poll_request[number_of_requests].fd = 0; //stdin
		poll_request[number_of_requests].events = POLLIN;
		poll(poll_request, number_of_requests + 1, -1); // -1 for wait infinit
		for (int i = 0; i < number_of_requests; i++) {
			if (poll_request[i].revents == POLLOUT && requests[i].status == wait_for_send)
				sent_header(i);
			else if (poll_request[i].revents == POLLIN)
				receve_http(i);
		}
		if (poll_request[number_of_requests].revents == POLLIN) {
			get_command();
			
		}
		free(poll_request);
		int temp = 0;
		while (temp < number_of_requests) {
			if (requests[temp].status == finish) {
				printf("download from %s to %s finished\n", requests[temp].request, requests[temp].file_path);
				stop(temp);
			}
			else
				temp++;
		}
		{

		}
	}

	return 0;
}