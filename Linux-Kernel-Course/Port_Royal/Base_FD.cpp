#include "Base_FD.hpp"
#include <unistd.h>

Base_FD::Base_FD(const int fd) : m_fd(fd) {}

Base_FD::~Base_FD()
{
    close(m_fd);
}

int Base_FD::get()
{
    return m_fd;
}