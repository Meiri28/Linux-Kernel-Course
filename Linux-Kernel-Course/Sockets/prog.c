#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/poll.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

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
	int bytes_left;
};

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


int main() {
	//struct pollfd p;
	//p.fd = 0;
	//p.events = POLLIN;
	//poll(&p, 1, -1);
	//char a;
	//read(0, &a, 1);
	//printf("%c", a);
	//printf("fd connected is %d\n", connect_http_with_domain("google.com"));
	int f = connect_http_with_domain("google.com");
	char* resuest = create_get_request("google.com", "/index.html");
	if (send(f, resuest, strlen(resuest), 0) == -1)
		printf("send failed with errno %d\n", errno);
	free(resuest);
	char header[4096];
	if(recv(f,&header, 4096,0) == -1)
		printf("recv failed with errno %d\n", errno);

	printf("%s", header);


	return 0;
}