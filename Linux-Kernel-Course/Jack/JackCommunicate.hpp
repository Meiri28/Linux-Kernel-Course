#pragma once
#include <string>

class JackCommunicate final
{
public:
	JackCommunicate(const std::string& ip);
	~JackCommunicate();

	std::string get_request();

	void send_response(const std::string& response);
private:
	int m_socket_fd;
};

