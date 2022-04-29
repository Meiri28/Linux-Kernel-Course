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
};

struct MyStruct
{
	char* request;
	char* file_path;
	int fd;
	enum request_status status;
	char headr[MAX_HTTP_HEADER_SIZE];
	int byte_upload;
	int byte_download_to_header;
	int bytes_download_to_body;
	char* body;
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
	close(requests[num].fd);
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

	return f;
}

void get_command() {
	char* line = NULL;
	int size;
	getline(&line, &size, stdin);
	if (strncmp(line, "show", sizeof(char) * strlen("show")) == 0) {
		show();
	} else if (strncmp(line, "stop", sizeof(char) * strlen("stop")) == 0) {
		stop(atoi(line + strlen("stop") + 1));
	} else if (strncmp(line, "start", sizeof(char) * strlen("start")) == 0) {
		requests = realloc(requests, sizeof(number_of_requests + 1));
		number_of_requests++;
		char* url = strstr(line, " ") + 1;
		char* endurl = strstr(url, " ");
		requests[number_of_requests - 1].request = malloc(sizeof(char) * (int)(endurl - url + 1));
		strncat(requests[number_of_requests - 1].request, url, endurl - url);
		requests[number_of_requests - 1].request[endurl - url] = '\0';
		requests[number_of_requests - 1].file_path = malloc(sizeof(char) * strlen(endurl + 1));
		strcat(requests[number_of_requests - 1].file_path, endurl + 1);
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

void receve_http(int num) 
{
	char header[4096];
	if(recv(requests[num].fd,&header, 4096,0) == -1)
		printf("recv failed with errno %d\n", errno);
	
	printf("%s", header);
}

int main() {
	running = 1;
	number_of_requests = 0;
	while (running) {
		//int *available_fd = malloc(sizeof(int) * (number_of_requests + 1));
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
	}
	
	//int f = connect_http_with_domain("google.com");
	//char* resuest = create_get_request("google.com", "/index.html");
	//if (send(f, resuest, strlen(resuest), 0) == -1)
	//	printf("send failed with errno %d\n", errno);
	//free(resuest);
	//char header[4096];
	//if(recv(f,&header, 4096,0) == -1)
	//	printf("recv failed with errno %d\n", errno);
	//
	//printf("%s", header);


	return 0;
}