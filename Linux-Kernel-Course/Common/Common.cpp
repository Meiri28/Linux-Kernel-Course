#include "Common.hpp"

#include <sys/socket.h>
#include <iostream>
#include <vector>

namespace common
{

std::string get_from_socket(const int socket_fd)
{
	constexpr int MAX_SIZE_LEN = 10;
	std::string size_buffer(MAX_SIZE_LEN, '\0');
	int recv_return_value = recv(socket_fd, const_cast<char*>(size_buffer.data()), MAX_SIZE_LEN, 0);
	if (recv_return_value == -1)
		std::cout << "recv error" << std::endl;
	else if (recv_return_value == 0)
		return "";
	int request_size = std::stoi(size_buffer);
	std::cout << "RECIVING " << request_size << std::endl;
	int start_of_request = size_buffer.find('\n') + 1;
	std::string request(request_size, '\0');
	request.insert(0, size_buffer, start_of_request);
	while (request.find('\0') < request_size) {
		int end_of_current_string = request.find('\0');
		recv_return_value = recv(socket_fd, const_cast<char*>(request.c_str()) + end_of_current_string, request_size - end_of_current_string, 0);
		if (recv_return_value == -1)
			std::cout << "recv error with request " << request << std::endl;
		else if (recv_return_value == 0)
			break;
	}
	
	return request;
}

void send_to_socket(int socket_fd, const std::string& data)
{
	std::string temp(data);
	constexpr int NO_FLAGS = 0;
	std::string payload_to_send;
	temp.shrink_to_fit();
	if (temp.find('\0') != std::string::npos)
		temp.resize(temp.find('\0'));
	std::cout << "SENDING " << temp.length() << std::endl;
	payload_to_send += std::to_string(temp.length());
	payload_to_send += '\n';
	payload_to_send += data;
	int sended = 0;
	while (sended < payload_to_send.length())
	{
		sended += send(socket_fd, payload_to_send.c_str() + sended, payload_to_send.length() - sended, NO_FLAGS);
	}
}

}