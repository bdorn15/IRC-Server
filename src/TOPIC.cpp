#include "Global.hpp"

/**
 * @brief Command sent by client to set/change channel topic.
 * @note IRC command: TOPIC <channel> [<topic>]
 */
void Server::TOPIC(Client& cl, const std::vector<std::string>& par)
{
	int retval = GOOD;
	std::string message;

	if (par.size() < 2)
	{
		message = buildMessage(461, cl, par.at(0), "", ""); //ERR_NEEDMOREPARAMS
		sendMessage(cl, message);
		return;
	}

	std::string ch_name = par.at(1);
	const std::string& src_nick = cl.getNick();
	Channel* ch = this->findChannel(ch_name);

	if (not ch) {
		retval = 403; //ERR_NOSUCHCHANNEL
	}
	else if (not ch->isChannelUser(src_nick)) {
		retval = 442; //ERR_NOTONCHANNEL
	}
	else if (par.size() == 2) {
		if (ch->getChannelTopic().empty()) {
			retval = 331; //RPL_NOTOPIC
		}
		else {
			retval = 332; //RPL_TOPIC
		}
	}
	else if (ch->getChannelModes().find("t") != std::string::npos) {
		if (not ch->isChannelOperator(src_nick))
			retval = 482; //ERR_CHANOPRIVSNEEDED
	}

	if (retval != GOOD) {
		if (retval == 332) {
			message = buildMessage(retval, cl, ch_name, ch->getChannelTopic(), "");
			sendMessage(cl, message);
			std::pair<std::string, std::time_t> set_time = ch->getChannelTopicInfo();
			const std::string& time = std::to_string(set_time.second);
			message = buildMessage(333, cl, ch_name, set_time.first, time); //RPL_TOPICWHOTIME
		}
		else {
			message = buildMessage(retval, cl, ch_name, "", "");
		}
		sendMessage(cl, message);
		return ;
	}

	std::string new_topic = par.at(2);
	ch->setChannelTopic(new_topic, src_nick, cl.getUsername(), cl.getHost());
	message = cl.getPrefix(); 
	message += MSG_TOPIC(ch_name, new_topic); 
	const std::vector<Client*> clients = ch->getChannelClients();
	for (Client* client : clients) {
		sendMessage(*client, message);
	} 
}


