#pragma once
#include <string>

namespace common
{

std::string get_from_socket(int socket_fd);

void send_to_socket(int socket_fd, const std::string& data);

};

