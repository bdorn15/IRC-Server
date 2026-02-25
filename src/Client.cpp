#include "Global.hpp"

/*==============================*/
/*	CONSTRUCTORS, DESTRUCTORS	*/
/*==============================*/

/**
 * @brief Constructor for client.
 * @note As it is, all pertinent variables are set here.
 * @details Initialising registration vectors and setting Client creation /
 * 			registration start time.
 */
Client::Client(int FD, const std::string& host)
	: clientFD_(FD), inBuffer_(""), clientHost_(host)
{
	clientReg_.passString.reserve(2);
	clientReg_.nickString.reserve(3);
	clientReg_.userString.reserve(5);

	clientReg_.passString.emplace_back("");
	clientReg_.nickString.emplace_back("");
	clientReg_.userString.emplace_back("");

	clientReg_.regStart = std::time(nullptr);
}

/*==============================*/
/*			GETTERS				*/
/*==============================*/

/**
 * @brief Return the FD for client communication.
 */
int Client::getFd() const
{
	return (clientFD_);
}

/**
 * @brief Return the buffer.
 */
std::string &Client::getInBuffer()
{
	return (inBuffer_);
}

/**
 * @brief Return the current nick of the client.
 */
std::string &Client::getNick(void)
{
	return (clientNick_);
}

/**
 * @brief Return the registered username of the client.
 */
std::string &Client::getUsername(void)
{
	return (clientUsername_);
}

/**
 * @brief Return the registered realname of the client.
 */
std::string &Client::getRealname(void)
{
	return (clientRealname_);
}

/**
 * @brief Return the registered host of the client.
 */
std::string &Client::getHost(void)
{
	return (clientHost_);
}

/**
 * @brief Fetch nick!~user@host combination, commonly required for message construction.
 * @note Includes colon : at the start.
 */
std::string Client::getPrefix()
{
	std:: string prefix = ":" + clientNick_ + "!~" + clientUsername_ + "@" + clientHost_;
	return (prefix);
}

/**
 * @brief Return as reference the map of client channels.
 * @details May not be useful.
 */
std::map<Channel*, bool> &Client::getChannelMap(void)
{
	return (clientChannels_);
}

/**
 * @brief Return the current list of channels the client is a member of, including
 * their membership prefix. Format:- :[prefix]<channel>{ [prefix]<channel>}
 * @details Returned string follows the form of the RPL_WHOISCHANNELS (319) message.
 * @note This only takes into account channel op mode, as that is all that is required
 * by the subject.
 */
std::string Client::getChannels(void)
{
	std::string tmpChanList;

	for (auto it = clientChannels_.begin(); it != clientChannels_.end(); ++it)
	{
		if (it->second)
			tmpChanList += "@";
		tmpChanList += it->first->getChannelName();
		tmpChanList += " ";
	}

	return (tmpChanList);
}

time_t Client::getConTime(void) const
{
	return (clientReg_.regStart);
}

std::string Client::getUserParam(void) const
{
	return (clientReg_.userString.at(3));
}

bool Client::isOpOfChan(Channel *channel)
{
	bool opStatus;

	try
	{
		opStatus = clientChannels_.at(channel);
	}
	catch(const std::out_of_range& e)
	{
		return (false);
	}
	
	if (opStatus == true)
		return (true);
	return (false);
}

/**
 * @brief Is the client registered with the server.
 * @note This can only be set once they have provided the password, a valid 
 * nick and username.
 */
bool Client::isRegistered(void)
{
	return (clientReg_.clientRegOK);
}

/**
 * @brief Has the client got a valid nickname.
 * @note This can only be set once they have provided the password.
 */
bool Client::hasSetNick(void)
{
	return (clientReg_.clientNickOK);
}

/**
 * @brief Has the client provided a valid password.
 * @note The client cannot do anything until the password has been given.
 */
bool Client::hasPassBeenGiven(void)
{
	return (clientReg_.clientPassOK);
}

/*==============================*/
/*			SETTERS				*/
/*==============================*/

/**
 * @brief Set the client's registration flag
 */
void Client::setRegistration(bool regValue)
{
	clientReg_.clientRegOK = regValue;
}

/**
 * @brief Set the client's password status.
 */
void Client::setPassGiven(bool passValue)
{
	clientReg_.clientPassOK = passValue;
}

/**
 * @brief Set the client's nick validity status.
 */
void Client::setNickStatus(bool nickValue)
{
	clientReg_.clientNickOK = nickValue;
}

/**
 * @brief Set whether the client's nick command was successful or not.
 */
void Client::setNickFailStatus(bool nickValue)
{
	clientReg_.clientNickFailed = nickValue;
}

/*==============================*/
/*		CHANNEL LIST			*/
/*==============================*/

void Client::addChannel(Channel *channelObj, bool operatorStatus)
{
	clientChannels_.insert(std::pair<Channel*, bool>(channelObj, operatorStatus));
}

void Client::removeChannel(Channel *channelObj)
{
	clientChannels_.erase(channelObj);
}
