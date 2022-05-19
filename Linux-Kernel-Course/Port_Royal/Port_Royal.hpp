#pragma once
#include "Client.hpp"

#include "Base_FD.hpp"

#include <vector>

class Port_Royal final
{
public:
    Port_Royal();
    ~Port_Royal() = default;

    void execute();

private:
    static int open_port_and_listen();

    std::vector<Client> m_connected_client;
    Base_FD m_socket_to_listen;
};

