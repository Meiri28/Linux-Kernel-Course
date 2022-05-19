#include "Port_Royal.hpp"

#include "../Common/Common.hpp"

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>

Port_Royal::Port_Royal() : m_socket_to_listen(open_port_and_listen()) {}

void Port_Royal::execute() 
{
    struct sockaddr_in their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(m_socket_to_listen.get(), (struct sockaddr*)&their_addr, &addr_size);
    std::cout << "accepted connection from " << inet_ntoa(their_addr.sin_addr) << std::endl;
    std::string command = "run echo hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello worldhello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world hello world ";
    
    common::send_to_socket(new_fd, command);
    std::string a = common::get_from_socket(new_fd);
    std::cout << "got from jack " << a << std::endl;
}

int Port_Royal::open_port_and_listen()
{
    struct addrinfo hints, * res;
    int sockfd, new_fd;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use IPv4 or IPv6, whichever
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // fill in my IP for me

    if (getaddrinfo(NULL, "1234", &hints, &res) != 0)
        std::cout << "getaddrinfo error" << std::endl;
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1)
        std::cout << "socket error" << std::endl;
    if (bind(sockfd, res->ai_addr, res->ai_addrlen) != 0)
        std::cout << "bind error" << std::endl;
    if (listen(sockfd, 10) != 0) // 10 is the maximum connected devices
        std::cout << "listen error" << std::endl;
    freeaddrinfo(res);

    return sockfd;
}