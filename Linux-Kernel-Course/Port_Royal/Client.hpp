#pragma once
#include "../Common/Base_FD.hpp"

#include <string>

class Client final
{
public:
    Client(int fd_to_get_client_from);
    ~Client() = default;

    int get_socket();
    std::string send_command(const std::string& command);
    std::string get_ip();

    Client(Client&) = delete;
    Client(Client&&) = default;
    void operator=(Client&) = delete;
    void operator=(Client&&) = delete;
private:
    Base_FD m_client_socket;
    std::string m_ip_address;
};

