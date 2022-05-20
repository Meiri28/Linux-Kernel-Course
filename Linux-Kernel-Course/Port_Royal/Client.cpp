#include "Client.hpp"

#include "../Common/Common.hpp"

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>

Client::Client(int fd_to_get_client_from): m_client_socket(-1)
{
    struct sockaddr_in their_addr;
    socklen_t addr_size = sizeof(their_addr);
    int new_fd = accept(fd_to_get_client_from, (struct sockaddr*)&their_addr, &addr_size);
    std::cout << "accepted connection from " << inet_ntoa(their_addr.sin_addr) << std::endl;
    m_ip_address = inet_ntoa(their_addr.sin_addr);
    m_client_socket.set(new_fd);
}

int Client::get_socket()
{
    return m_client_socket.get();
}

void Client::send_command(const std::string& command) 
{
    common::send_to_socket(m_client_socket.get(), command);
    std::cout << common::get_from_socket(m_client_socket.get()) << std::endl;
}

std::string Client::get_ip()
{
    return m_ip_address;
}