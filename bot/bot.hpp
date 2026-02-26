#pragma once

#include <string>
class bot
{
	private:

		std::string password;
		int port;
		std::string hostname;
		int fd;
		bool _running;

	public:

		bot() = delete;
		~bot() = default;
		bot(const bot &other) = delete;
		bot& operator=(const bot &other) = delete;
		
		bot(std::string password, int port, std::string hostname);
		int connectToServer();
		bool authenticate();
		void run();
		void stop();
		bool command(std::string &line, std::string &nick);
		void closeFd();
		void handleMessage(std::string &line);
};