#include "Global.hpp"

/**
 * @brief Command sent by client to leave the channel.
 * @note IRC command: PART <channel>{,<channel>} [<reason>]
 * @note message is sent for every channel given as parameter
 */
void Server::PART(Client& cl, const std::vector<std::string>& par)
{
	int retval = GOOD;
	std::string message;

	if (par.size() < 2) {
		message = buildMessage(461, cl, par.at(0), "", ""); //ERR_NEEDMOREPARAMS
		sendMessage(cl, message);
		return;
	}

	std::stringstream ss(par.at(1));
	std::string target_channel;
	const std::string& nick = cl.getNick();
	std::string reason = par.size() > 2 ? par.at(2) : "no reason";

	while (std::getline(ss, target_channel, ',')) {
		Channel* target = findChannel(target_channel);
		if (not target) {
			retval = 403; //ERR_NOSUCHCHANNEL
		}
		else if (not target->isChannelUser(nick)) {
			retval = 442; //ERR_NOTONCHANNEL
		}
		else {
			retval = GOOD;
		}

		if (retval != GOOD) {
			message = buildMessage(retval, cl, target_channel, "", "");
			sendMessage(cl, message);
		}
		else {
			message = cl.getPrefix();
			message += MSG_PART(target_channel, reason);
			const std::vector<Client*> clients = target->getChannelClients();
			for (Client* client : clients) {
				sendMessage(*client, message);
			} 
			cl.removeChannel(target);
			target->removeChannelUser(&cl);
			if (target->getChannelUserCount() == 0)
				removeChannel(target_channel);
		}
	}	
}
