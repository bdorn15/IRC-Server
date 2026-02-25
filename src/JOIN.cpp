#include "Global.hpp"

/**
 * @brief Helper to handle channel join messages.
 */
static void joinedChan(Channel &channelObj, Server &serverObj, Client &clientObj, const std::string &chanName)
{
	//Server confirmation message, no numeric.
	std::string clientId;
	std::string replyMessage;
	replyMessage = clientObj.getPrefix() + " JOIN " + chanName + "\r\n";
	serverObj.sendMessage(clientObj, replyMessage);

	//RPL_TOPIC message sent if a topic is set. Nothing otherwise.
	//If implemented, send topic setter and time (RPL_TOPICWHOTIME)
	if (channelObj.getChannelTopic() != "")
	{
		//RPL_TOPIC
		serverObj.doMessage(332, clientObj, chanName, channelObj.getChannelTopic(), "");
		//RPL_TOPICWHOTIME
		serverObj.doMessage(333, clientObj, chanName, channelObj.getChannelTopicInfo().first, std::to_string(channelObj.getChannelTopicInfo().second));
	}

	//Channel nick list and end of nick list messages.
	std::vector<std::string> tmpVec = {"NAMES", chanName};
	serverObj.NAMES(clientObj, tmpVec);
}

/**
 * @brief Helper to handle informing channel about JOINs.
 */
static void informChan(Channel &channelObj, Server &serverObj, Client &clientObj, const std::string &chanName)
{
	if (channelObj.getChannelUserCount() < 2)
		return ;
	
	//Server confirmation message, no numeric.
	std::string clientId;
	std::string replyMessage;
	replyMessage = clientObj.getPrefix() + " JOIN " + chanName + "\r\n";
	
	std::vector<Client*> tmpClientList = channelObj.getChannelClients();
	//for (auto itVec = tmpClientList.begin(); itVec != tmpClientList.end(); ++itVec)
	for (auto elemVec : tmpClientList)
	{
		if (elemVec->getNick() != clientObj.getNick())
			serverObj.sendMessage(*elemVec, replyMessage);
	}
}

/**
 * @brief Command send by client to join a channel(s).
 * @note IRC command: JOIN <comma sep list of channels> <comma sep list of keys>
 * @note -Channel names need the correct prefix (#&+).
 * @note -Key position in key list needs to be same as channel position in channel list.
 * @note Server reply Opt 1: :<nick>!<user>@<host> JOIN <#channel> * :<real name>
 * @note Server reply Opt 2: :<nick> JOIN <#channel> * :<real name>
 * @details Can be refactored quite a bit.
 * @attention Function allocates memory with: new Channel
 */
int Server::JOIN(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	//Empty command should ask for more parameters.
	int retval = GOOD;
	if (cmdVec.size() < 2)
	{
		retval = 461;	//ERR_NEEDMOREPARAMS
	}
	else
	{
		//Place channels and keys (if present) in a map for ease of use
		std::unordered_map<std::string, std::string> chanMap;
		std::vector<std::string> leftVec = xsvSplit(cmdVec.at(1), ',');
		int numChans = leftVec.size();

		if (cmdVec.size() > 2)
		{
			std::vector<std::string> rightVec = xsvSplit(cmdVec.at(2), ',');
			for (int index = 0; index < numChans; ++index)
				chanMap.insert(std::pair<std::string, std::string>(leftVec[index], rightVec[index]));
		}
		else
		{
			for (int index = 0; index < numChans; ++index)
				chanMap.insert(std::pair<std::string, std::string>(leftVec[index], ""));
		}

		//for (auto elem : chanMap)
		//	std::cout << "Left: " << elem.first << " | " << "Right: " << elem.second << std::endl;

		//Run through channel/key map handling each request individually
		for (auto itChanMap = chanMap.begin(); itChanMap != chanMap.end(); ++itChanMap)
		{
			//std::cout << "Channel: " << itChanMap->first << " | " << "Key: " << itChanMap->second << std::endl;
			
			//Check channel name validity.
			//Wrong channel type, too long channel name and illegal characters
			//should cause error
			if (itChanMap->first[0] != '#' || itChanMap->first.length() > CHAN_LEN_MAX
				|| itChanMap->first.find_first_of(CHAN_ILLEGAL_CHARS) != std::string::npos)
			{
				//ERR_BADCHANMASK
				doMessage(476, clientObj, itChanMap->first, "", "");
				continue ;
			}

			auto itServChan = _channels.find(itChanMap->first);
			if (itServChan != _channels.end())
			{
				//If channel is INVITE only, take precedence over all other restructions.
				if (itServChan->second->getChannelModes().find('i') != std::string::npos)
				{
					if (isInXSV(clientObj.getNick(), itServChan->second->getChannelInviteList(), ','))
					{
						itServChan->second->addChannelClient(&clientObj);
						clientObj.addChannel(itServChan->second, false);
						joinedChan(*itServChan->second, *this, clientObj, itChanMap->first);
						informChan(*itServChan->second, *this, clientObj, itChanMap->first);
					}
					else
					{
						//ERR_INVITEONLYCHAN
						doMessage(473, clientObj, itChanMap->first, "", "");
						continue ;
					}
				}
				else
				{
					//If channel has a KEY, check and act accordingly.
					if (!itServChan->second->isChannelKey(itChanMap->second))
					{
						//ERR_BADCHANNELKEY
						doMessage(475, clientObj, itChanMap->first, "", "");
						continue ;
					}
					//If channel has a LIMIT check and act accordingly.
					else if (itServChan->second->getChannelUserLimit() <= itServChan->second->getChannelUserCount() && itServChan->second->getChannelUserLimit() != 0)
					{
						//ERR_CHANNELISFULL
						doMessage(471, clientObj, itChanMap->first, "", "");
						continue ;
					}

					itServChan->second->addChannelClient(&clientObj);
					clientObj.addChannel(itServChan->second, false);
					joinedChan(*itServChan->second, *this, clientObj, itChanMap->first);
					informChan(*itServChan->second, *this, clientObj, itChanMap->first);
				}
			}
			else
			{
				Channel *newChan = new Channel(itChanMap->first, &clientObj);
				setChannel(newChan);
				joinedChan(*newChan, *this, clientObj, itChanMap->first);
			}
		}
	}

	if (retval != GOOD)
		doMessage(retval, clientObj, cmdVec.at(0), "", "");

	return (retval);
}
