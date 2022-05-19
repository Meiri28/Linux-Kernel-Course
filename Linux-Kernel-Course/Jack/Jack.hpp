#pragma once
#include <string>

class Jack final
{
public:
	static void execute(const std::string& ip);

	static void stop();

	static std::string run_shell(const std::string& command);

	// deleted function
	Jack() = delete;
	~Jack() = delete;
private:
	static bool m_jack_should_stop;
};

