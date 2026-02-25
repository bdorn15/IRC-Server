#include "Global.hpp"

/**
 * @brief Command send by client before closing their connection.
 * @note IRC command: JOIN <reason>
 * @note Server message: :<nick>!<user>@<host> QUIT :Quit: <reason>
 */
void Server::QUIT(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	std::map<Channel*, bool> tmpChanMap = clientObj.getChannelMap();
	std::string quitMessage = "Client quit";

	if (cmdVec.size() > 1)
	{
		if (cmdVec.at(1)[0] == ':' && cmdVec.at(1).size() > 1)
			quitMessage = cmdVec.at(1).substr(1, cmdVec.at(1).size());
		else
			quitMessage = cmdVec.at(1);
	}

	//std::cout << "QUIT message 1: " << quitMessage << std::endl;
	quitMessage = clientObj.getPrefix() + MSG_QUIT(quitMessage);
	//std::cout << "QUIT message2: " << quitMessage << std::endl;
	
	for (auto itChans = tmpChanMap.begin(); itChans != tmpChanMap.end(); ++itChans)
	{
		std::vector<Client*> tmpChanClientVec = itChans->first->getChannelClients();
		for (auto elUsers : tmpChanClientVec)
		{
			if (clientObj.getFd() != elUsers->getFd())
				send(elUsers->getFd(), quitMessage.c_str(), quitMessage.size(), 0);
		}
		clientObj.removeChannel(itChans->first);
		itChans->first->removeChannelUser(&clientObj);
		if (itChans->first->getChannelUserCount() == 0) {
			const std::string ch_name = itChans->first->getChannelName();
			removeChannel(ch_name);
		}
	}
}
