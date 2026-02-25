#include "Global.hpp"

/*
	Mostly functioning regex to validate client IRC messages.
^(?<command>[\w]+)(?<params>(?: [\w\!*#&.,+-@]+)*)(?:(?<endparam> :.*)?)$

//For all messages add to beginning after ^ ... even less functioning.
(?:(?<tags>@[\S]+) )?(?:(?<source>:[\w.!-@]+) )?
*/

/**
 * @brief Split Buffer_ into individual IRC messages.
 */
std::vector<std::string> Client::bufferSplitMessages_(void)
{
	std::vector<std::string> tmpVec;
	size_t index = 0;
	size_t found = 0;
	while (found != std::string::npos)
	{
		found = inBuffer_.find(MSG_DELIM, index);
		if (found == std::string::npos)
			break;
		tmpVec.emplace_back(inBuffer_.substr(index, found - index));
		inBuffer_ = inBuffer_.substr(found + 2, 100);
	}

	return (tmpVec);
}

/**
 * @brief Parse the inBuffer_. Send each CRLF terminated line to checkMessage().
 * clear() buffer once done and buffer ends with message delimiter.
 * @note True does not mean that the buffer had no erroneous commands, just that
 * the commands did not justify disconnection.
 * @return True: Client can remain connected
 * @return False: Client must be disconnected
 */
bool Client::processBuffer(Server &serverObj)
{
	if (inBuffer_.length() < 1)
		return (true);

	//std::cout << ">>>> inBuffer start >>\n" << inBuffer_ << "\n<< inBuffer end -----" << std::endl;

	auto tmpVec = bufferSplitMessages_();
	for (auto it = tmpVec.begin(); it != tmpVec.end(); ++it)
	{
		int tmpRes = checkMessageValidity_(*this, serverObj, it->data());
		if (tmpRes != GOOD)
		{
			it->erase();
			break;
		}
		//std::cout << "[ In] [FD: " << clientFD_ << "] [" << it->data() << "] [Res: " << tmpRes << "] ";

		auto tmpCmdVec = splitMessage_(it->data());

		if (!clientReg_.clientRegOK)
			tmpRes = doRegistration_(serverObj, tmpCmdVec);
		else
			tmpRes = executeMessage_(*this, serverObj, tmpCmdVec);

		//std::cout << "[Res: " << tmpRes << "]\n";

		//A retval of 666 means terminate the connection.
		if (tmpRes == 666 || tmpCmdVec.at(0) == "QUIT")
			return (false);
	}

	if (inBuffer_.find(MSG_DELIM) != std::string::npos)
		inBuffer_.clear();
	return (true);
}

/**
 * @brief Checks a client message from processBuffer() for format and length.
 * If found to be incorrect outputs an error message and returns non-GOOD retval.
 * @note Client messages should never have @ (tags) or : (source) leading parts.
 */
int Client::checkMessageValidity_(Client &clientObj, Server &serverObj, const std::string &clientMessage)
{
	//std::cout << "\n[ In] [FD: " << clientFD_ << "] [" << clientMessage << "] [Len: " << clientMessage.length() << "]\n";

	int retval = GOOD;
	if (clientMessage.empty())
		retval = 400;	//ERR_UNKNOWNERROR
	else if (clientMessage[0] == '@' || clientMessage[0] == ':')
		retval = 400;	//ERR_UNKNOWNERROR
	else if (clientMessage.length() > MSG_LEN_MAX)
		retval = 417;	//ERR_INPUTTOOLONG

	if (retval != GOOD)
	{
		std::string newMessage = serverObj.buildMessage(retval, clientObj, "", "", "Badly formed client message.");
		serverObj.sendMessage(clientObj, newMessage);
		return (retval);
	}

	return (retval);
}

/**
 * @brief Split a given IRC message into its component parts. Concatenate
 * colon prefixed message into single element. Capitalise first word (command).
 * @note MSG format: <command> <parameters> :<long param>
 */
std::vector<std::string> Client::splitMessage_(const std::string &clientMessage)
{
	std::stringstream tmpSS(clientMessage);
	std::string tmpWord;
	std::vector<std::string> tmpVec;
	std::string tmpLongMsg;

	while (tmpSS >> tmpWord)
	{
		if (tmpWord[0] == ':' || tmpLongMsg.length() != 0)
			tmpLongMsg += tmpWord + " ";
		else
			tmpVec.emplace_back(tmpWord);
	}

	if (tmpLongMsg.length() != 0)
		tmpVec.emplace_back(tmpLongMsg);

	std::transform(tmpVec.at(0).begin(), tmpVec.at(0).end(), tmpVec.at(0).begin(), ::toupper);

	return (tmpVec);
}

/**
 * @brief Hash command name. Helper for executeMessage_
 * @details Stolen wholesale from stackoverflow.
 * https://stackoverflow.com/questions/650162/
 */
constexpr unsigned int hash(const char *s, int off) //REMOVE off = 0 SINCE IT GAVE COMPILATION ERROR
{                    
    return !s[off] ? 5381 : (hash(s, off+1)*33) ^ s[off];                           
}

/**
 * @brief Checks a client message from processBuffer(). Executes relevant command
 * if the message is formatted correctly.
 * @note Client messages should never have @ (tags) or : (source) leading parts.
 * This is different to server messages.
 * @details Using a hasing function to check input. Not super happy with this.
 */
int Client::executeMessage_(Client &clientObj, Server &serverObj, std::vector<std::string> &tmpVec)
{
	int retval = GOOD;

	switch (hash(tmpVec.at(0).c_str()))
	{
	case hash("PASS"):
		retval = serverObj.PASS(clientObj, tmpVec);
		break;
	case hash("NICK"):
		retval = serverObj.NICK(clientObj, tmpVec);
		break;
	case hash("USER"):
		retval = serverObj.USER(clientObj, tmpVec);
		break;
	case hash("JOIN"):
		retval = serverObj.JOIN(clientObj, tmpVec);
		break;
	case hash("PART"):
		serverObj.PART(clientObj, tmpVec);
		break;
	case hash("TOPIC"):
		serverObj.TOPIC(clientObj, tmpVec);
		break;
	case hash("INVITE"):
		serverObj.INVITE(clientObj, tmpVec);
		break;
	case hash("KICK"):
		serverObj.KICK(clientObj, tmpVec);
		break;
	case hash("MODE"):
		serverObj.MODE(clientObj, tmpVec);
		break;
	case hash("PRIVMSG"):
		serverObj.PRIVMSG(tmpVec, tmpVec.at(tmpVec.size() - 1), clientFD_);
		break;
	case hash("NAMES"):
		serverObj.NAMES(clientObj, tmpVec);
		break;
	case hash("WHO"):
		serverObj.WHO(clientObj, tmpVec);
		break;
	case hash("WHOIS"):
		serverObj.WHOIS(clientObj, tmpVec);
		break;
	case hash("LUSERS"):
		serverObj.LUSERS(clientObj);
		break;
	case hash("LIST"):
		serverObj.LIST(clientObj);
		break;
	case hash("MOTD"):
		serverObj.MOTD(clientObj);
		break;
	case hash("PING"):
		serverObj.PONG(clientObj, tmpVec);
		break;
	case hash("QUIT"):
		serverObj.QUIT(clientObj, tmpVec);
		break;
	default:
		retval = 421;	//ERR_UNKNOWNCOMMAND
		serverObj.doMessage(retval, clientObj, tmpVec.at(0), "", "");
		break;
	}

	return (retval);
}
