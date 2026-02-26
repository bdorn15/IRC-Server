#include "bot.hpp"
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <random>
#include <vector>
#include <sstream>

bot::bot(std::string password, int port, std::string hostname) : password(password), port(port), hostname(hostname), fd(-1), _running(true) {} 


int bot::connectToServer()
{
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
    {
        std::cerr << "Error: Could not create socket\n";
        return -1;
    }
	struct sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = inet_addr(hostname.c_str());

	if (connect(fd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
	{
		std::cerr << "Error: Connection failed\n";
        close(fd);
        return -1;
	}
	std::cout << "✅ Connected to " << hostname << ":" << port << " (fd=" << fd << ")\n";
    return fd;
}

bool bot::authenticate()
{
    std::string passCmd = "PASS " + password + "\r\n";
    std::string nickCmd = "NICK bot\r\n";
    std::string userCmd = "USER bot 0 bot :bot\r\n";

    if (send(fd, passCmd.c_str(), passCmd.size(), 0) < 0)
        return std::cerr << "Error: Failed to send PASS command\n", false;
    if (send(fd, nickCmd.c_str(), nickCmd.size(), 0) < 0)
        return std::cerr << "Error: Failed to send NICK command\n", false;
    if (send(fd, userCmd.c_str(), userCmd.size(), 0) < 0)
        return std::cerr << "Error: Failed to send USER command\n", false;

    std::cout << "✅ Authentication commands sent for bot\n";
    return true;
}

void bot::run()
{
	char buffer[512];
	std::string leftover;

	while(_running)
	{
		std::memset(buffer, 0, sizeof(buffer));
		ssize_t bytes = recv(fd,buffer, sizeof(buffer) - 1, 0);
		if (bytes <= 0)
		{
			std::cout << "Connection closed or error\n";
			break;
		}
		leftover += buffer;

		size_t pos;
		while((pos = leftover.find("\r\n")) != std::string::npos)
		{
			std::string line = leftover.substr(0, pos);
            leftover.erase(0, pos + 2);

            handleMessage(line);
		}
	}
	close(fd);
}

static std::string getNick(std::string &line)
{
	std::string nick;

	size_t pos = line.find('!');

	if (pos != std::string::npos)
		nick = line.substr(1, pos - 1);
	else
		nick = "";

	return nick;
}

static int roll(int check)
{
	static std::random_device rd;
    static std::mt19937 gen(rd());
	
	if (check)
	{
    	std::uniform_int_distribution<> dist(0, 100);
		return dist(gen);
	}
	else
	{
		std::uniform_int_distribution<> dist(0, 1);
		return dist(gen);
	}
}


bool bot::command(std::string &line, std::string &nick)
{
	std::istringstream iss(line);
	std::vector<std::string> tokens;
	std::string token;

	while (iss >> token) 
	{
        tokens.push_back(token);
    }
	if (tokens.size() < 4)
    	return false;
	if (tokens[1] != "PRIVMSG")
		return false;
	if (tokens[3] != "!roll" && tokens[3] != "!coin" && tokens[3] != ":!coin" && tokens[3] != ":!roll")
		return false;
	if (tokens[3] == "!roll" || tokens[3] == ":!roll")
	{
		std::cout << "token: " << tokens[3] << std::endl;
		int result = roll(true);
		std::string num = std::to_string(result);
		if (tokens.size() > 4)
		{
			std::string msg = "PRIVMSG " + tokens[4] + " :" + nick + " rolls: " + num + "\r\n";
			send(fd, msg.c_str(), msg.size(), 0);
		}
		else
		{
			std::string msg = "PRIVMSG " + nick + " :" + nick + " rolls: " + num + "\r\n";
			send(fd, msg.c_str(), msg.size(), 0);
		}
	}
	else if (tokens[3] == "!coin" || tokens[3] == ":!coin")
	{
		std::string coin;
		int result = roll(false);
		if (result == 1)
			coin = "Heads!";
		else
			coin = "Tails!";
		if (tokens.size() > 4)
		{
			std::string msg = "PRIVMSG " + tokens[4] + " :" + nick + " tosses a coin... " + coin + "\r\n";
			send(fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
		}
		else
		{
			std::string msg = "PRIVMSG " + nick + " :" + nick + " tosses a coin... " + coin + "\r\n";
			send(fd, msg.c_str(), msg.size(), MSG_NOSIGNAL);
		}
	}
	return true;
}


void bot::handleMessage(std::string &line)
{
	if (line.rfind(":NANA", 0) == 0)
    	return;
	else
	{
		std::cout << "line: " << line << std::endl;
		std::string nick = getNick(line);
		if (nick.empty())
			std::cout << "Error\n";
		if (!command(line, nick))
		{
			std::string msg = "PRIVMSG " + nick + " :Unknown command. Available commands: !roll [channel], !coin [channel]\r\n";  
			send(fd, msg.c_str(), msg.size(), 0);
		}
	}	
}

void bot::stop()
{
	_running = false;
}

void bot::closeFd()
{
	close(fd);
}
