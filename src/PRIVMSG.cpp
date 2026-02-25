#include "Global.hpp"

void Server::PRIVMSG(std::vector<std::string> args, std::string message, int senderFD)
{
	std::string target;
	Client *sender = _clients[senderFD];
	std::set<std::string> seen;
	std::string server_name = SERVER_NAME;

	if (args.size() < 2)
    {
        std::string err = ":" + server_name + " 411 " + sender->getNick().c_str() + " :No recipient given (PRIVMSG)\r\n";
		send(senderFD, err.c_str(), err.size(), MSG_NOSIGNAL);
        return;
    }
	if (args.size() < 3)
    {
        std::string err = ":" + server_name + " 412 " + sender->getNick() + " :No text to send\r\n";
		send(senderFD, err.c_str(), err.size(), MSG_NOSIGNAL);
        return;
    }
	std::stringstream ss(args[1]);
	while (std::getline(ss, target, ','))
	{
		if (target.empty() || !seen.insert(target).second)
			continue;

		if (target[0] == '#' || target[0] == '&' || target[0] == '+' || target[0] == '!')
		{
			//handle sent to channel
			std::string name = target;
			std::map<std::string, Channel*>::iterator it = _channels.find(name);
			if (it == _channels.end())
			{
				std::string errMsg = ERR_NOSUCHNICK(target);
				send(senderFD, errMsg.c_str(), errMsg.size(), MSG_NOSIGNAL);
				continue;
			}
			else
			{
				Channel *chan = it->second;
				std::vector<Client*> _users = chan->getChannelClients();
				std::vector<Client*>::iterator uit = _users.begin();
				for(; uit != _users.end(); ++uit)
				{
					int fd = (*uit)->getFd();
					if (fd == senderFD)
						continue;
					std::string	sendMessage = sender->getPrefix() + " PRIVMSG " + target + " " + message + "\r\n";
					send(fd, sendMessage.c_str(), sendMessage.size(), MSG_NOSIGNAL);
				}
			}
		}
		else
		{
			//handle send to client
			std::map<int, Client*>::iterator it = _clients.begin();
			for (; it != _clients.end(); ++it)
			{
				if (it->second->getNick() == target)
				{
					int fd = it->second->getFd();
			 		std::string	sendMessage = sender->getPrefix() + " PRIVMSG " + target + " " + message + "\r\n";
					send(fd, sendMessage.c_str(), sendMessage.size(), MSG_NOSIGNAL);
					break;
				}
			}
			if (it == _clients.end())
			{
				std::string errMsg = ERR_NOSUCHNICK(target);
				send(senderFD, errMsg.c_str(), errMsg.size(), MSG_NOSIGNAL);
			}
		}
	}
}