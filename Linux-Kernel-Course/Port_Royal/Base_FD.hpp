#pragma once

class Base_FD final
{
public:
    Base_FD(int fd);
    ~Base_FD();

    int get();
    void set(int fd);

    Base_FD(Base_FD&) = delete;
    Base_FD(Base_FD&&);
    void operator=(Base_FD&) = delete;
    void operator=(Base_FD&&) = delete;
private:
    int m_fd;
};

