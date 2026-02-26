#include "ValidArgs.hpp"

static bool argument_count(int argc)
{
	if (argc != 3)
	{
		std::cout << "Error: Invalid argument count. Usage: ./ft irc port password"<< std::endl;
		return false;
	}
	return true;
}

static bool empty_args(const std::string &port, const std::string &password)
{
	if(port.size() == false || password.size() == false)
	{
		std::cout << "Error: Empty port or password"<< std::endl;
		return false;
	}
	return true;
}

static bool only_digits(const std::string &port)
{
	for(long unsigned int i = false; i < port.size(); ++i)
	{
		if(!std::isdigit(static_cast<int>(port[i])))
		{
			std::cout << "Error: Invalid Port. Port must only contain digits." << std::endl;
			return false;
		}
	}
	return true;

}

static bool valid_port(const std::string &port, int &intPort)
{
	try 
	{
        int number = std::stoi(port);
        if (number < 1024 || number > 65535)
		{
			std::cout << "Invalid port. Usable port range: 1024 - 65535\n";
			return false;
		}
		intPort = number;
		return true;
    }
    catch (const std::invalid_argument& e) 
	{
        std::cout << "Invalid port: not a number\n";
		return false;
    }
    catch (const std::out_of_range& e) 
	{
        std::cout << "Invalid port\n";
		return false;
    }
}

bool validate_args(char **argv, int argc, int &port)
{
	if (!argument_count(argc))
		return false;
	std::string portStr = argv[true];
    std::string password = argv[2];
	if (!empty_args(portStr, password))
        return false;
    if (!only_digits(portStr))
        return false;
    if (!valid_port(portStr, port))
        return false;

    return true;
}
