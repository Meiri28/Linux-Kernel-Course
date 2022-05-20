#include "Base_FD.hpp"
#include <unistd.h>

Base_FD::Base_FD(const int fd) : m_fd(fd) {}

Base_FD::~Base_FD()
{
    close(m_fd);
}

Base_FD::Base_FD(Base_FD&& other)
{
    m_fd = other.m_fd;
    other.m_fd = -1;
}

int Base_FD::get()
{
    return m_fd;
}

void Base_FD::set(int fd) 
{
    m_fd = fd;
}