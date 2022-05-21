#include "File.hpp"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <iostream>
#include <unistd.h>

File::File(const std::string& file_path) : m_fd(open(file_path.data(), O_RDWR)) {
    if (m_fd.get() == -1) {
        std::cout << "open error " << file_path << std::endl;
    }
}

std::string File::read_whole_file()
{
    std::string result(100, '\0');
    while (int a = read(m_fd.get(), const_cast<char*>(result.data()) + result.find_first_of('\0'), result.capacity() - result.find_first_of('\0'))) {
        if (a == -1)
            std::cout << "read error " << std::endl;
        result.resize(result.length() + 100);
    }

    return result;
}

void File::write_data(const std::string& data)
{
    int written = 0;
    while (written < data.length())
    {
        written += write(m_fd.get(), data.data() + written, data.length() - written);
    }
}
