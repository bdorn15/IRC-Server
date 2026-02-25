#include "Global.hpp"

/**
 * @brief Fetch all nicks from the server _client list.
 * @note As we do not have user modes nor channel modes p or s. This is an allowed
 * option for NAMES usage.
 * @details This is done slightly differently to protocol, as it just lists all 
 * users but not all channels.
 */
std::string Server::fetchAllNicks(void)
{
	std::string tmpList;

	for (auto iter = _clients.begin(); iter != _clients.end(); ++iter)
	{
		tmpList += iter->second->getNick() + " ";
	}

	return (tmpList);
}

/**
 * @brief Fetch the list of users and their channel membership prefixes.
 * @note Command: /NAMES <channel>,<channel>....
 * @details In 353, '=' stands for public channels. This is hardcoded as that
 * is the only kind of channel we have.
 */
void Server::NAMES(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	if (cmdVec.size() == 1)
	{
		std::string tmpAllNames = fetchAllNicks();
		return ;
	}

	//rearrange comma separated channels into a vector
	std::vector<std::string> chanVec = xsvSplit(cmdVec.at(1), ',');

	//iterate through vector, checking each given channel
	for (auto itChanVec = chanVec.begin(); itChanVec != chanVec.end(); ++itChanVec)
	{
		auto itServChan = _channels.find(itChanVec->data());
		if (itServChan != _channels.end())
		{
			std::string nickList = itServChan->second->getChannelUserList();
			doMessage(353, clientObj, itChanVec->data(), "=", nickList);
			doMessage(366, clientObj, itChanVec->data(), "", "");
		}
		else
		{
			doMessage(366, clientObj, itChanVec->data(), "", "");
		}
	}
}

/**
 * @brief Fetch the list of channel users, their prefixes, hosts, usernames, servers
 * and other sundry information specified in RPL_WHOREPLY
 * @note Command: /WHO <channel>
 * @details This is a more complicated command with masks etc. but we do not
 * need to worry about that I think.
 */
void Server::WHO(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	int retval = GOOD;
	if (cmdVec.size() < 2)
	{
		retval = 461; //ERR_NEEDMOREPARAMS
		doMessage(retval, clientObj, cmdVec.at(0), "", "");
		return ;
	}

	std::string whoMessage;
	std::string tmpFlags = "";
	if (cmdVec.at(1)[0] == '#')
	{
		Channel *tmpChan = findChannel(cmdVec.at(1));
		if (tmpChan != nullptr)
		{
			for (auto elem : tmpChan->getChannelClients())
			{
				tmpFlags = "";
				if (elem->isOpOfChan(tmpChan) == true)
					tmpFlags = "@";

				whoMessage = buildMsgHeader(352, clientObj);
				whoMessage += RPL_WHOREPLY(cmdVec.at(1), elem->getUsername(), elem->getHost(), std::string(SERVER_NAME), elem->getNick(), tmpFlags, elem->getRealname());
				sendMessage(clientObj, whoMessage);
			}
		}
	}
	else
	{
		Client *tmpClient = findClient(cmdVec.at(1));
		if (tmpClient != nullptr)
		{
			std::string tmpArbChan = "*";
			whoMessage = buildMsgHeader(352, clientObj);
			whoMessage += RPL_WHOREPLY(tmpArbChan, tmpClient->getUsername(), tmpClient->getHost(), std::string(SERVER_NAME), tmpClient->getNick(), tmpFlags, tmpClient->getRealname());
			sendMessage(clientObj, whoMessage);
		}
	}

	if (retval == GOOD)
		doMessage(315, clientObj, cmdVec.at(1), "", "");
}

/**
 * @brief Fetches and sends server statistics. Part of the standard registration
 * messages.
 * @note Command: /LUSERS
 * @note As we are single server only, redundancy has been omitted.
 */
void Server::LUSERS(Client &clientObj)
{
	//RPL_LUSERCLIENT
	doMessage(251, clientObj, std::to_string(_clients.size()), "0", "1");
	//RPL_LUSERCHANNELS
	doMessage(254, clientObj, std::to_string(_channels.size()), "", "");
}

/**
 * @brief Returns relevant whois RPL lines for our use case.
 * @note Command: /WHOIS <nick>
 */
void Server::WHOIS(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	if (cmdVec.size() < 2)
	{
		//ERR_NEEDMOREPARAMS
		doMessage(461, clientObj, cmdVec.at(0), "", "");
		return ;
	}

	std::string tmpNick;
	if (cmdVec.size() == 2)
		tmpNick = cmdVec.at(1);
	else
		tmpNick = cmdVec.at(2);

	Client *foundClient = findClient(tmpNick);
	if (foundClient == nullptr)
	{
		//ERR_NOSUCHNICK
		doMessage(401, clientObj, tmpNick, "", "");
		return ;
	}
	else
	{
		//RPL_WHOISUSER
		std::string whoisMessage = buildMsgHeader(311, clientObj);
		whoisMessage += RPL_WHOISUSER(foundClient->getNick(), foundClient->getUsername(), foundClient->getHost(), foundClient->getRealname());
		sendMessage(clientObj, whoisMessage);

		//RPL_WHOISCHANNELS
		doMessage(319, clientObj, foundClient->getNick(), foundClient->getChannels(), "");
		//RPL_WHOISSERVER
		doMessage(312, clientObj, foundClient->getNick(), SERVER_NAME, SERVER_INFO);
		//RPL_ENDOFWHOIS
		doMessage(318, clientObj, foundClient->getNick(), "", "");
	}
}

/**
 * @brief List all channels currently on the server
 * @note Command: /LIST
 * @details For testing, only lists channels and has no parameters
 */
void Server::LIST(Client &clientObj)
{
	//RPL_LISTSTART
	doMessage(321, clientObj, "", "", "");

	//RPL_LIST
	for (auto it = _channels.begin(); it != _channels.end(); ++it)
		doMessage(322, clientObj, it->first, std::to_string(it->second->getChannelUserCount()), it->second->getChannelTopic());

	//RPL_LISTEND
	doMessage(323, clientObj, "", "", "");
}