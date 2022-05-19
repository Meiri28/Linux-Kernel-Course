#pragma once
class Base_FD final
{
public:
    Base_FD(int fd);
    virtual ~Base_FD();
    int get();
private:
    int m_fd;
};

