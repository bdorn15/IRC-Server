#include "Global.hpp"

/**
 * @brief PING command.
 * @details I have a feeling this may be useful for dealing with unrsponsive
 * clients. As of now, not used.
 */
int Server::PING(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	int retval = GOOD;
	if (cmdVec.size() < 2)
	{
		retval = 461;	//ERR_NEEDMOREPARAMS
		doMessage(retval, clientObj, cmdVec.at(0), "", "");
		return (retval);
	}

	std::string pingMesage = ":" + std::string(SERVER_NAME) + " PING";
	pingMesage += " " + std::string(SERVER_NAME) + " :" + cmdVec.at(1) + "\r\n";
	sendMessage(clientObj, pingMesage);

	return (retval);
}

/**
 * @brief PONG command to reply to client PING commands.
 * @note Reply with any parameter sent with the original PING.
 */
void Server::PONG(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	std::string pongMesage = ":" + std::string(SERVER_NAME) + " PONG";
	if (cmdVec.size() < 2)
		sendMessage(clientObj, pongMesage + " PONG\r\n");
	else
	{
		pongMesage += " " + std::string(SERVER_NAME) + " " + cmdVec.at(1) + "\r\n";
		sendMessage(clientObj, pongMesage);
	}
}
