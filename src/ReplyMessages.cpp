#include "Global.hpp"

/**
 * @brief Fill a string with the message that coincides with a given numeric.
 * @details Varaidic would be good here, but maybe that is too risky, depending
 * on who evaluates us.
 */
static std::string fillMsgBody(const int &numeric, Client &clientObj, const std::string &arg1, const std::string &arg2, const std::string &arg3)
{
	//temporary until i can fetch from elsewhere
	std::string date = "today";

	switch (numeric)
	{
	case 1:
		return (RPL_WELCOME(std::string(NETWORK_NAME), clientObj.getNick()));
	case 2:
		return (RPL_YOURHOST(std::string(SERVER_NAME), std::string(SERVER_VERSION)));
	case 3:
		return (RPL_CREATED(date));
	case 4: //modes could be brought from a variable elsewhere maybe
		return (RPL_MYINFO(std::string(SERVER_NAME), std::string(SERVER_VERSION), "iop", "itkl"));
	case 5: //modes could be brought from a variable elsewhere maybe
		return (RPL_ISUPPORT(arg1));
	case 251:
		return (RPL_LUSERCLIENT(arg1, arg2, arg3));
	case 254:
		return (RPL_LUSERCHANNELS(arg1));
	case 301:
		return (RPL_AWAY(arg1, arg2));
	//case 311: //Special case, handle elsewhere.
	//	return (RPL_WHOISUSER(arg1, arg2, arg3, arg4));
	case 312:
		return (RPL_WHOISSERVER(arg1, arg2, arg3));
	case 315:
		return (RPL_ENDOFWHO(arg1));
	case 318:
		return (RPL_ENDOFWHOIS(arg1));
	case 319:
		return (RPL_WHOISCHANNELS(arg1, arg2));
	case 321:
		return (RPL_LISTSTART);
	case 322:
		return (RPL_LIST(arg1, arg2, arg3));
	case 323:
		return (RPL_LISTEND);
	case 324:
		return (RPL_CHANNELMODEIS(arg1, arg2, arg3));
	case 329:
		return (RPL_CREATIONTIME(arg1, arg2));
	case 331:
		return (RPL_NOTOPIC(arg1));
	case 332:
		return (RPL_TOPIC(arg1, arg2));
	case 333:
		return (RPL_TOPICWHOTIME(arg1, arg2, arg3));
	case 341:
		return (RPL_INVITE(arg1, arg2));
	//case 352: //Special case, handle elsewhere.
	//	return (RPL_WHOREPLY(arg1, arg2, arg3, arg4, arg5));
	case 353:
		return (RPL_NAMREPLY(arg1, arg2, arg3));
	case 366:
		return (RPL_ENDOFNAMES(arg1));
	case 372:
		return (RPL_MOTD(arg1));
	case 375:
		return (RPL_MOTDSTART(arg1));
	case 376:
		return (RPL_ENDOFMOTD);
	case 400:
		return (ERR_UNKNOWNERROR(arg1, arg2, arg3));
	case 401:
		return (ERR_NOSUCHNICK(arg1));
	case 402:
		return (ERR_NOSUCHSERVER(arg1));
	case 403:
		return (ERR_NOSUCHCHANNEL(arg1));
	case 404:
		return (ERR_CANNOTSENDTOCHAN(arg1));
	case 405:
		return (ERR_TOOMANYCHANNELS(arg1));
	case 409:
		return (ERR_NOORIGIN);
	case 411:
		return (ERR_NORECIPIENT(arg1));
	case 412:
		return (ERR_NOTEXTTOSEND);
	case 417:
		return (ERR_INPUTTOOLONG);
	case 421:
		return (ERR_UNKNOWNCOMMAND(arg1));
	case 422:
		return (ERR_NOMOTD);
	case 431:
		return (ERR_NONICKNAMEGIVEN);
	case 432:
		return (ERR_ERRONEUSNICKNAME(arg1));
	case 433:
		return (ERR_NICKNAMEINUSE(arg1));
	case 441:
		return (ERR_USERNOTINCHANNEL(arg1, arg2));
	case 442:
		return (ERR_NOTONCHANNEL(arg1));
	case 443:
		return (ERR_USERONCHANNEL(arg1, arg2));
	case 451:
		return (ERR_NOTREGISTERED);
	case 461:
		return (ERR_NEEDMOREPARAMS(arg1));
	case 462:
		return (ERR_ALREADYREGISTERED);
	case 464:
		return (ERR_PASSWDMISMATCH);
	case 471:
		return (ERR_CHANNELISFULL(arg1));
	case 473:
		return (ERR_INVITEONLYCHAN(arg1));
	case 474:
		return (ERR_BANNEDFROMCHAN(arg1));
	case 475:
		return (ERR_BADCHANNELKEY(arg1));
	case 476:
		return (ERR_BADCHANMASK(arg1));
	case 482:
		return (ERR_CHANOPRIVSNEEDED(arg1));
	case 501:
		return (ERR_UMODEUNKNOWNFLAG);
	case 502:
		return (ERR_USERSDONTMATCH);
	default:
		return ("Default message return. Should not happen.");
	}

	return ("This should happen even less than the default!");
}

/**
 * @brief Constructs the first part of every message sent to the client.
 * Format: :<server> <numeric code> <nick>
 */
std::string Server::buildMsgHeader(const int &numeric, Client &clientObj)
{
	std::string tmpNum;
	if (numeric < 10)
		tmpNum = "00";
	else if (numeric < 100)
		tmpNum = "0";
	tmpNum += std::to_string(numeric);

	std::string tmpNick;
	if (clientObj.getNick().empty())
		tmpNick = " * ";
	else
		tmpNick = clientObj.getNick();

	std::string msgStart;
	msgStart = MSG_HEADER(std::string(SERVER_NAME), tmpNum, tmpNick);
	
	return (msgStart);
}

/**
 * @brief Builds the whole message.
 */
std::string Server::buildMessage(const int &numeric, Client &clientObj, const std::string &arg1, const std::string &arg2, const std::string &arg3)
{
	std::string tmpMessage;
	tmpMessage = buildMsgHeader(numeric, clientObj);
	tmpMessage += fillMsgBody(numeric, clientObj, arg1, arg2, arg3);

	//std::cout << "tmpMessage: " << tmpMessage << std::endl;

	return (tmpMessage);
}

/** 
 * @brief Helper to form messages.
*/
void Server::doMessage(const int &numeric, Client &clientObj, const std::string &arg1, const std::string &arg2, const std::string &arg3)
{
	std::string tempMessage;
	tempMessage = buildMessage(numeric, clientObj, arg1, arg2, arg3);
	sendMessage(clientObj, tempMessage);
}

/*==============================*/
/*		LOCAL-REMOTE COMMS		*/
/*==============================*/

/**
 * @brief Use send() to forward server message to client.
 * @details returns number of bytes sent, if ever needed
 */
int Server::sendMessage(Client &clientObj, const std::string &message)
{
	int bytesSent;
	bytesSent = send(clientObj.getFd(), message.c_str(), message.length(), MSG_NOSIGNAL);

	if (bytesSent == -1)
	{
		std::cerr << "NOTE: send() error, bytesSent: " << bytesSent << std::endl;
	}

	return (bytesSent);
}
