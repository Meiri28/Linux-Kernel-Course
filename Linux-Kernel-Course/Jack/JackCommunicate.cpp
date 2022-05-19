#include "JackCommunicate.hpp"

#include "../Common/Common.hpp"

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

JackCommunicate::JackCommunicate(const std::string& ip) 
{
	struct addrinfo hints;
	struct addrinfo* res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if (getaddrinfo(ip.c_str(), "1234", &hints, &res) != 0) {
		
	}
	m_socket_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	connect(m_socket_fd, res->ai_addr, res->ai_addrlen);
	freeaddrinfo(res);
}

JackCommunicate::~JackCommunicate()
{
	close(m_socket_fd);
}

std::string JackCommunicate::get_request()
{
	return common::get_from_socket(m_socket_fd);
}

void JackCommunicate::send_response(const std::string& response)
{
	common::send_to_socket(m_socket_fd, response);
}