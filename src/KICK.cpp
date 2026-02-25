#include "Global.hpp"

/**
 * @brief Command sent by client to kick a user out from the Channel
 * @note IRC command: KICK <channel> <user> *( "," <user> ) [<comment>]
 */
void Server::KICK(Client& cl, const std::vector<std::string>&par)
{
	int retval = GOOD;
	std::string message;

	if (par.size() < 3) {
		message = buildMessage(461, cl, par.at(0), "", ""); //ERR_NEEDMOREPARAMS
		sendMessage(cl, message);
		return;
	}
	
	std::string ch_name = par.at(1);
	std::string target_names = par.at(2);
	Channel* ch = this->findChannel(ch_name);
	const std::string& src_nick = cl.getNick();

	if (not ch) {
		retval = 403; //ERR_NOSUCHCHANNEL
	}
	else if (not ch->isChannelUser(src_nick)) {
		retval = 442; //ERR_NOTONCHANNEL
	}
	else if (not ch->isChannelOperator(src_nick)) {
		retval = 482; //ERR_CHANOPRIVSNEEDED
	}

	if (retval != GOOD) {
		message = buildMessage(retval, cl, ch_name, "", "");
		sendMessage(cl, message);
		return;
	}

	std::string comment = par.size() > 3 ? par.at(3) : "";
	std::stringstream ss(target_names);
	std::string target_nick;

	while (std::getline(ss, target_nick, ',')) {
		Client* target = findClient(target_nick);
		if (target == nullptr) {
			retval = 401; //ERR_NOSUCHNICK
		}
		else {
			if (ch->isChannelUser(target_nick)) {
				retval = GOOD;
			}
			else {
				retval = 441; //ERR_USERNOTINCHANNEL;
			}
		}

		if (retval != GOOD) {
			message = buildMessage(retval, cl, ch_name, "", "");
			sendMessage(cl, message);
		}
		else {
			message = cl.getPrefix();
			if (comment.empty()) {
				message += MSG_KICK(ch_name, target_nick, target_nick);
			}
			else {
				message += MSG_KICK(ch_name, target_nick, comment);
			}
			const std::vector<Client*> clients = ch->getChannelClients();
			for (Client* client : clients) {
				sendMessage(*client, message);
			} 
			target->removeChannel(ch);
			ch->removeChannelUser(target);
			if (ch->getChannelUserCount() == 0)
				removeChannel(ch_name);
		}
	}
}

