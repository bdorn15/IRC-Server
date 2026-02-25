#include "Global.hpp"

/**
 * @brief Command sent by client to add user to channels invite list.
 * @note IRC command: INVITE <nickname> <channel>.
 * @note only one target per invite request
 */
void Server::INVITE(Client& cl, const std::vector<std::string>& par)
{
	int retval = GOOD;
	std::string message;
	
	if (par.size() < 3) {
		message = buildMessage(461, cl, par.at(0), "", ""); //ERR_NEEDMOREPARAMS
		sendMessage(cl, message);
		return;
	}

	const std::string& src_nick = cl.getNick();
	const std::string& target_nick = par.at(1);
	std::string ch_name = par.at(2);
	Client* cl_target = this->findClient(target_nick);
	Channel* ch = this->findChannel(ch_name);

	if (not ch) {
		retval = 403; //ERR_NOSUCHCHANNEL
	}
	else if (not ch->isChannelUser(src_nick)) {
		retval = 442; //ERR_NOTONCHANNEL
	}
	else if (ch->getChannelModes().find("i") != std::string::npos) {
		if (not ch->isChannelOperator(src_nick))
			retval = 482; //ERR_CHANOPRIVSNEEDED
	}

	if (retval == GOOD) {
		if (cl_target == nullptr) {
			retval = 401; //ERR_NOSUCHNICK
		}
		else if (ch->isChannelUser(target_nick)) {
			retval = 443; //ERR_USERONCHANNEL
		}
	}

	if (retval != GOOD) {
		message = buildMessage(retval, cl, ch_name, "", "");
		sendMessage(cl, message);
		return;
	}

	ch->addChannelInvite(target_nick);
	message = buildMessage(341, cl, ch_name, target_nick, ""); //RPL_INVITE
	sendMessage(cl, message);

	std::string invitation = cl.getPrefix();
	invitation += MSG_INVITE(target_nick, ch_name); 
	sendMessage(*cl_target, invitation);
}

