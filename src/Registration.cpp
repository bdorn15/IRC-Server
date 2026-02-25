#include "Global.hpp"

/**
 * @brief Process registration commands.
 * @note PASS has to go first. Then NICK or USER. Registration cannot complete
 * until all commands are processed and completed with no errors.
 * @note This is the only series of messages where the server will enforce a certain order.
 * @return True: Client can remain connected
 * @return False: Client must be disconnected
 */
int Client::doRegistration_(Server &serverObj, std::vector<std::string> &cmdVec)
{
	int retval = GOOD;
	if (clientReg_.regBufferOK != 7)
	{
		if (cmdVec[0] == "PASS" && clientReg_.passString[0].size() == 0)
		{
			clientReg_.passString = cmdVec;
			clientReg_.regBufferOK += 4;
			retval = executeMessage_(*this, serverObj, clientReg_.passString);
		}
		else if (cmdVec[0] == "NICK" && clientReg_.nickString[0].size() == 0)
		{
			clientReg_.nickString = cmdVec;
			clientReg_.regBufferOK += 2;
		}
		else if (cmdVec[0] == "USER" && clientReg_.userString[0].size() == 0)
		{
			clientReg_.userString = cmdVec;
			clientReg_.regBufferOK += 1;
		}
	}
	
	if (clientReg_.regBufferOK == 7)
	{
		if (clientReg_.clientPassOK)
		{
			if (!clientReg_.clientNickOK)
			{
				if (!clientReg_.clientNickFailed)
					retval = executeMessage_(*this, serverObj, clientReg_.nickString);
				else
				{
					if (cmdVec[0] == "NICK")
						retval = executeMessage_(*this, serverObj, cmdVec);
				}
			}
			
			if (clientReg_.clientNickOK)
			{
				if (!clientReg_.clientRegOK)
				{
					retval = executeMessage_(*this, serverObj, clientReg_.userString);
					if (retval !=  GOOD)
					{
						clientReg_.userString[0].clear();
						clientReg_.regBufferOK -= 1;
					}
				}
			}
		}
	}

	return (retval);
}

/**
 * @brief Command sent by client to verify password.
 * @details Needs 1 parameter, if missing error. If client registered, error.
 * If password not correct, error.
 * @details If required by server it must precede any other client command.
 * @note IRC Command: PASS <password>
 * @note Anything wrong with the password should result in client disconnect.
 */
int Server::PASS(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	int retval = GOOD;
	if (clientObj.isRegistered())
		retval = 462;	//ERR_ALREADYREGISTERED
	else if (clientObj.hasPassBeenGiven() == true || cmdVec.size() > 2)
		retval = 400;	//ERR_UNKNOWNERROR
	else if (cmdVec.size() < 2)
		retval = 461;	//ERR_NEEDMOREPARAMS
	else if (cmdVec.at(1) != _password)
		retval = 464;	//ERR_PASSWDMISMATCH

	if (retval != GOOD)
	{
		std::string newMessage = buildMessage(retval, clientObj, cmdVec.at(0), "", "Password already sent or password command malformed");
		sendMessage(clientObj, newMessage);
		if (retval != 462)
			retval = 666;
	}
	else
		clientObj.setPassGiven(true);
	
	return (retval);
}

/**
 * @brief Command sent by client to set username.
 * @note RFC2812 command: USER <username> <mode> * <realname>
 * @note RFC1459 command: USER <username> <hostname> <servername> <realname>
 * @note - <mode>, <hostname> and <servername> can be ignored in our case I think.
 * As we do not support user modes or server/server communication.
 * @note - Last stage of registration. No direct server reply. However this triggers
 * the registration messages (RPL: 001-005) plus others. See docs.
 * @note -- An invalid username results in connection being terminated.
 */
int Server::USER(Client &clientObj, const std::vector<std::string> &cmdVec)
{
	int retval = GOOD;
	std::string err400 = "";
	if (clientObj.isRegistered())
		retval = 462;	//ERR_ALREADYREGISTERED
	else if (clientObj.hasPassBeenGiven() == false)
	{
		retval = 400;	//ERR_UNKNOWNERROR
		err400 = "Password not yet sent.";
	}
	else if (cmdVec.size() < 5)
		retval = 461;	//ERR_NEEDMOREPARAMS
	else if (cmdVec.at(4).length() < 1)
		retval = 461;	//ERR_NEEDMOREPARAMS
	else if (cmdVec.at(2) != "0" || (cmdVec.at(3) != "*" && cmdVec.at(3) != "bot"))
		retval = 461;	//ERR_NEEDMOREPARAMS
	else if (!isUsernameValid_(cmdVec.at(1)))
	{
		retval = 400;	//ERR_UNKNOWNERROR
		err400 = "Invalid username given.";
	}

	std::string userMessage;
	if (retval != GOOD)
	{
		userMessage = buildMessage(retval, clientObj, cmdVec.at(0), "", err400);
		sendMessage(clientObj, userMessage);
	}
	else
	{
		clientObj.getUsername() = cmdVec.at(1);
		std::string tmpRN = cmdVec.at(4);
		if (cmdVec.at(4).size() == 1)
			tmpRN = ":" + cmdVec.at(4);
		else if (cmdVec.at(4)[0] != ':' && cmdVec.at(4).size() > 1)
			tmpRN = ":" + cmdVec.at(4);
		clientObj.getRealname() = tmpRN;
		clientObj.setRegistration(true);
		confirmReg_(clientObj);
	}

	if (err400 == "Invalid username given.")
		retval = 666;

	return (retval);
}

/**
 * @brief Sends the server Message of the Day, if the file is present.
 * @note If MOTD file is not present, send 422.
 */
void Server::MOTD(Client &clientObj)
{
	std::ifstream motdFile(MOTD_FILE);

	if (motdFile.is_open())
	{
		//RPL_MOTDSTART
		doMessage(375, clientObj, SERVER_NAME, "", "");
		std::string line;
		//RPL_MOTD
		while (std::getline(motdFile, line))
			doMessage(372, clientObj, line, "", "");
		//RPL_ENDOFMOTD
		doMessage(376, clientObj, "", "", "");
		motdFile.close();
	}
	else //ERR_NOMOTD
		doMessage(422, clientObj, "", "", "");
}

/**
 * @brief Send RPL 001 to 005 and MOTD if present. Notifies client registration
 * is complete.
 */
int Server::confirmReg_(Client &clientObj)
{
	std::cout << "Client registered, FD: " << clientObj.getFd() << std::endl;
	
	std::string messageRPL;
	for (auto i = 1; i <= CLIENT_CONF_CNT; ++i)
	{
		messageRPL = buildMessage(i, clientObj, "", "", "");
		//std::cout << "Confirmation message [" << i << "]: " << messageRPL << std::endl;
		sendMessage(clientObj, messageRPL);
	}

	std::string tokens;
	tokens = "CHANMODES=" + std::string(ISUPPORT_CHANMODES);
	tokens += " CHANNELLEN=" + std::to_string(CHAN_LEN_MAX);
	tokens += " USERLEN=" + std::to_string(USER_LEN_MAX);
	tokens += " NICKLEN=" + std::to_string(NICK_LEN_MAX);
	
	messageRPL = buildMessage(5, clientObj, tokens, "", "");
	//std::cout << "RPL_ISUPPORT message: " << messageRPL << std::endl;
	sendMessage(clientObj, messageRPL);

	LUSERS(clientObj);
	MOTD(clientObj);

	return (GOOD);
}
