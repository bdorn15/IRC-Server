#include "Global.hpp"

/**
 * @brief Checks a potential nickname does not break any validity rules.
 */
bool Server::isNickValid_(const std::string &newNick)
{
	if (newNick.length() > NICK_LEN_MAX)
		return (false);
	if (std::isdigit(newNick[0]))
		return (false);
	if (newNick.find_first_of(NICK_ILLEGAL_CHARS) != std::string::npos)
		return (false);

	return (true);
}

/**
 * @brief Checks a potential nickname is not currently in use.
 */
bool Server::isNickAvailable_(const std::string &newNick)
{
	for (auto it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second->getNick() == newNick)
			return (false);
	}

	return (true);
}

/**
 * @brief Checks a potential username does not break any validity rules.
 * @note Length of 30 is just a placeholder, replace with global setting value.
 * first_not_of may be better. Depending on what we choose.
 * Some servers close the connection if usernames contain certain characters.
 * Others silently remove or change them to _, there seems to be minimal 
 * 	documentation about how to handle this.
 */
bool Server::isUsernameValid_(const std::string &username)
{
	if (username.length() > USER_LEN_MAX)
		return (false);
	if (username.find_first_of(UNAME_ILLEGAL_CHARS) != std::string::npos)
		return (false);

	return (true);
}
