#include "Common.hpp"

#include <sys/socket.h>
#include <iostream>
#include <vector>

namespace common
{

std::string get_from_socket(const int socket_fd)
{
	int msg_size = 0;
	int recv_return_value = recv(socket_fd, &msg_size, sizeof(msg_size), 0);
	if (recv_return_value == -1)
		std::cout << "recv error" << std::endl;
	else if (recv_return_value == 0)
		return "";
	std::cout << "RECIVING " << msg_size << std::endl;
	std::string request(msg_size, '\0');
	while (request.find_first_of('\0') < msg_size) {
		int end_of_current_string = request.find_first_of('\0');
		recv_return_value = recv(socket_fd, const_cast<char*>(request.c_str()) + end_of_current_string, msg_size - end_of_current_string, 0);
		if (recv_return_value == -1)
			std::cout << "recv error with request " << request << std::endl;
		else if (recv_return_value == 0)
			break;
	}
	
	return request;
}

void send_to_socket(int socket_fd, const std::string& data)
{
	std::string payload_to_send(data);
	constexpr int NO_FLAGS = 0;
	payload_to_send.shrink_to_fit();
	if (payload_to_send.find('\0') != std::string::npos)
		payload_to_send.resize(payload_to_send.find('\0'));
	const int msg_size = payload_to_send.length();
	std::cout << "SENDING " << payload_to_send.length() << std::endl;
	int temp = send(socket_fd, &msg_size, sizeof(msg_size), NO_FLAGS);
	if (temp == -1) {
		std::cout << " send error" << std::endl;
		return;
	}
	payload_to_send += data;
	int sended = 0;
	while (sended < msg_size)
	{
		int temp = send(socket_fd, payload_to_send.c_str() + sended, msg_size - sended, NO_FLAGS);
		if (temp == -1) {
			std::cout << " send error" << std::endl;
			break;
		}
		sended += temp;
	}
}

}