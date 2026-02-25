#include "Global.hpp"

/**
 * @brief Command sent by client to set/change nickname.
 * @note IRC command: NICK <newNick>
 * @note Server reply Opt 1: :<oldNick>!<user>@<host> NICK :<newNick>
 * @note Server reply Opt 2: :<oldNick> NICK :<newNick>
 * @details Older protocol had a second parameter of <hopcount>.
 */
int Server::NICK(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	int retval = GOOD;
	std::string tmpNick = "";
	if (cmdVec.size() < 2)
		retval = 431;	//ERR_NONICKNAMEGIVEN
	else if (cmdVec.size() > 2)
		retval = 400;	//ERR_UNKNOWNERROR
	else if (!isNickValid_(cmdVec.at(1)))
	{
		retval = 432;	//ERR_ERRONEUSNICKNAME
		tmpNick = cmdVec.at(1);
	}
	else if (!isNickAvailable_(cmdVec.at(1)) ||(cmdVec.at(1) == "bot" && clientObj.getUserParam() != "bot"))
		retval = 433;	//ERR_NICKNAMEINUSE
	
	std::string nickMessage;
	if (retval != GOOD)
	{
		nickMessage = buildMessage(retval, clientObj, tmpNick, "", "Nick command malformed");
		sendMessage(clientObj, nickMessage);
		clientObj.setNickFailStatus(true);
	}
	else
	{
		if (!clientObj.getNick().empty())
			nickMessage = nickMessage + clientObj.getPrefix() + " NICK " + ":" + cmdVec.at(1) + "\r\n";
		
		sendMessage(clientObj, nickMessage);
		clientObj.getNick() = cmdVec.at(1);
		clientObj.setNickStatus(true);
		clientObj.setNickFailStatus(false);
	}

	return (retval);
}
