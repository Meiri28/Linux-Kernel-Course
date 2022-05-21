#pragma once
#include "Base_FD.hpp"

#include <string>

class File final
{
public:
    File(const std::string& file_path);
    ~File() = default;

    std::string read_whole_file();
    void write_data(const std::string& data);

    File(File&) = delete;
    File(File&&) = delete;
    void operator=(File&) = delete;
    void operator=(File&&) = delete;
private:
    Base_FD m_fd;
};