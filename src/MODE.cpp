#include "Global.hpp"

/*
 * @brief parse the modestring got from client
 * @note returned string will always start with '+' or '-'
 * @note invalid mode will always return nullptr
 * @note there can only be one 't', 'i', '-l' and '-k' per sign
 * 		- e.g. '/mode +tttii-kkttttkll -> '+ti-ktl'
 * @note there can be any number of '+l', '+k' and 'o' per sign
 * 		- e.g "+lllkkkooo-ooo" -> "+lllkkkooo-ooo"
 */
int parse_modestring(const std::vector<std::string>& par, const std::string& modestring, std::string& parsed)
{
	constexpr std::string_view MODES = "itkol";
	parsed += modestring[0] == '-' ? "-" : "+";
	char sign = parsed[0]; 
	int needed_args = 0;

	for (size_t i = 0; modestring[i]; i++) {
		char ch = modestring[i];

		if (ch == '+' || ch == '-') {
			if (sign != ch) {
				sign = ch;
				parsed += ch;
			}
			continue;
		}

		if (MODES.find(ch) == std::string_view::npos)
			return 472; //ERR_UNKNOWNMODE

		if (ch != 'o') {
			if (ch == 't' || ch == 'i' || sign == '-') {
				size_t signPos = parsed.rfind(sign);
				if (parsed.find(ch, signPos + 1) != std::string::npos)
					continue;
			}
		}
		parsed += ch;
		if (ch == 'o' || (sign == '+' && (ch == 'l' || ch == 'k')))
			needed_args++;
	}

	if (needed_args == 0)
		return GOOD;

	int par_count = par.size();
	if (par_count < 4)
		return 461; //ERR_NEEDMOREPARAMS
	
	par_count -= 3; //remove the MODE, channel and modestring parameters
	return needed_args <= par_count ? GOOD : 461;
}

void handleKeyMode(const std::string& key, char cur_sign, Channel& ch, std::vector<std::string>& args, std::string& last_key);
int handleOperatorMode(const std::string& target_nick, char cur_sign, Server& serv, Channel& ch, std::vector<std::string>& args);
/**
 * @brief Command setn by client to change the channels modes
 * @note IRC command: MODE <channel> [<modestring> [<mode__args>...]]
 * @note if no parameters given return a list of set mode
 * @rules
 * 	1. if + or - is not given + is presumed
 * 	2. if an invalid mode is given -> error
 * 	3. if only 1 valid mode is given -> all errors can be returned
 * 	4. if more than 1 valid modes are given -> most errors are silenced
 * 	5. 't' and 'i'
 * 		5.1 can only be set if unset
 * 		5.2 can only be unset if set
 * 		5.3 if not set/unset silently continue
 * 	6. 'l'
 * 		6.1 can be set multiple times if parameter given
 * 			6.1.1 as a response the last parameter is returned, e.g. '/mode +lll hello 3 4' -> MODE +lll 4 4 4
 * 			6.1.2 if invalid parameter given -> silently consume the parameter
 * 			6.1.3 if no parameter given -> error
 * 		6.2 can only be unset if set
 * 		6.3 if not set/unset silently continue
 * 	7. 'k'
 * 		7.1 if no parameter given -> error
 * 		7.2 can be set multiple times if parameter is given (i.e. consumes all the parameters)
 * 			7.2.1 all the parameters are accepted
 * 			7.2.2 as a response the last parameter is returned, e.g. '/mode +kkk key1 key2 key3' -> MODE +kkk key3 key3 key3
 * 		7.3 can only be unset if set and parameter is given
 * 			7.3.1 all the parameters are accepted
 * 			7.3.2 if unset all the preceding sets or unsets are changed to '*'
 * 	8. 'o'
 * 		8.1 if no parameters given -> error
 * 		8.2 other errors are given/silenced according to rules 3 and 4
 * 	9. modestring is formatted according to previous rules like this:
 * 		9.1 there can only be one 't', 'i', '-l' and '-k' per sign
 * 			- '/mode +tttii-kkttttkll -> '+ti-ktl'
 * 		9.2 there can be any number of '+l', '+k' and 'o' per sign
 * 			- '/mode +lllkkkooo-ooo' -> '+lllkkkooo-ooo'
 */
void Server::MODE(Client& cl, const std::vector<std::string>& par)
{
	int retval = GOOD;
	std::string message;

	if (par.size() < 2) {
		message = buildMessage(461, cl, par.at(0), "", ""); //ERR_NEEDMOREPARAMS
		sendMessage(cl, message);
		return;
	}

	std::string ch_name = par.at(1);
	Channel* ch = this->findChannel(ch_name);
	const std::string& src_nick = cl.getNick();
	std::string modestring;

	if (not ch) {
		retval = 403; //ERR_NOSUCHCHANNEL
	}
	else if (not ch->isChannelUser(src_nick)) {
		retval = 442; //ERR_NOTONCHANNEL
	}
	else if (par.size() < 3) {
		retval = 324; //RPL_CHANNELMODEIS
	}
	else { 
		modestring = par.at(2);
		if (modestring.empty()) {
			retval = 324; //RPL_CHANNELMODEIS
		}
		else if (modestring.find_first_not_of("+-") == std::string::npos) {
			return; //modestring only contains + and - -> return silently
		}
	}
								  
	std::string parsed_modes;
	if (retval == GOOD) {
		if (not ch->isChannelOperator(src_nick)) {
			retval = 482; //ERR_CHANOPRIVSNEEDED
		}
		else {
			retval = parse_modestring(par, modestring, parsed_modes);
		}
	}

	if (retval != GOOD) {
		if (retval == 324) {
			const std::string& mods = ch->buildChannelArgs();
			message = buildMessage(retval, cl, ch_name, ch->getChannelModes(), mods);
			sendMessage(cl, message);
			const std::string& time = std::to_string(ch->getChannelCreationTime());
			message = buildMessage(329, cl, ch_name, time, ""); //RPL_CREATIONTIME
		}
		else if (retval == 472) {
			size_t pos = modestring.find_first_not_of("+-itkol");
			message = ERR_UNKNOWNMODE(src_nick, std::string(SERVER_NAME), modestring[pos]); //472
		}
		else {
			message = buildMessage(retval, cl, ch_name, "", "");
		}
		sendMessage(cl, message);
		return;
	}

	std::vector<std::string> args; //place to store used args and placeholders
	char cur_sign;
	size_t mode_count = parsed_modes.size();
	std::string final_modes;
	unsigned int last_limit; //stores the last limit that will replace all the other limits given
	std::string last_key; //stores the last key that will replace all preceding placeholders
	size_t cur_arg = 3;

	for (size_t i = 0; parsed_modes[i]; i++) {
		char cur_mode = parsed_modes[i];

		if (cur_mode == '-' || cur_mode == '+') {
			cur_sign = cur_mode;
		}
		else if (cur_mode == 'o') {
			retval = handleOperatorMode(par.at(cur_arg++), cur_sign, *this, *ch, args);
			if (retval != GOOD) {
				if (mode_count == 2) //if mode count == 2 -> only 1 arg given and error is returned for all the errors
					break;
				retval = GOOD;
				continue;
			}
		}
		else if (cur_sign == '-') {
			if (cur_mode == 'i' || cur_mode == 't' || cur_mode == 'l') {
				if (!ch->isChannelMode(cur_mode))
					continue;
			}
			else if (cur_mode == 'k') {
				if (!ch->isChannelMode('k'))
					continue;
				handleKeyMode("*", cur_sign, *ch, args, last_key);
			}
			ch->removeChannelMode(cur_mode);
		}
		else {
			if (cur_mode == 'i' || cur_mode == 't') {
				if (ch->isChannelMode(cur_mode))
					continue;
			}
			else if (cur_mode == 'l') {
				try {
					unsigned int limit = std::stoi(par.at(cur_arg++));
					last_limit = limit;
				} catch (const std::exception& e) {
					continue;
				}
				args.push_back("placeholder for l");
				ch->setChannelUserLimit(last_limit);
			}
			else if (cur_mode == 'k') {
				handleKeyMode(par.at(cur_arg++), cur_sign, *ch, args, last_key);
			}
			ch->addChannelMode(cur_mode);
		}
		final_modes += cur_mode;
	}

	if (retval != GOOD) {
		message = buildMessage(retval, cl, par.at(0), "", "");
		sendMessage(cl, message);
		return;
	}

	if (final_modes.empty() || final_modes.find_first_not_of("+-") == std::string::npos) //if nothing has been changed don't response
		return;
	std::string message_args = "";
	for (std::string& str : args) {
		if (str == "placeholder for k") {
			message_args += last_key;
		}
		else if (str == "placeholder for l") {
			message_args += std::to_string(last_limit);
		}
		else {
			message_args += str;
		}
		message_args += " ";
	}
	if (message_args.size() > 1)
		message_args.pop_back(); //remove extra space
	while (!final_modes.empty() && (final_modes.back() == '-' || final_modes.back() == '+'))
		final_modes.pop_back();
	if (final_modes.size() > 2 && (final_modes.at(1) == '-' || final_modes.at(1) == '+')) {
		size_t first_mode = final_modes.find_first_not_of("+-");
		final_modes.erase(0, first_mode - 1);
	}
	message = cl.getPrefix();
	message += MSG_MODE(ch_name, final_modes, message_args);

	const std::vector<Client*> clients = ch->getChannelClients();
	for (Client* client : clients)
		sendMessage(*client, message);
}

void handleKeyMode(const std::string& key, char cur_sign, Channel& ch, std::vector<std::string>& args, std::string& last_key)
{
	if (cur_sign == '-') {
		//if keys were given earlier -> overwrite them with '*'
		for (std::string& str : args) {
			if (str == "placeholder for k")
				str = key;
		}
		args.push_back(key);
	}
	else {
		last_key = key;
		args.push_back("placeholder for k");
		ch.setChannelKey(key);
	}
}

int handleOperatorMode(const std::string& target_nick, char cur_sign, Server& serv, Channel& ch, std::vector<std::string>& args)
{
	Client* target = serv.findClient(target_nick);
	if (target == nullptr) {
		return 401; //ERR_NOSUCHNICK
	}
	else if (not ch.isChannelUser(target_nick)) {
		return 441; //ERR_USERNOTINCHANNEL;
	}

	args.push_back(target_nick);
	std::map<Channel*, bool> channel = target->getChannelMap();
	if (cur_sign == '-') {
		ch.removeChannelOperatorRights(target_nick);
		channel[&ch] = false;
	}
	else {
		ch.addChannelOperatorRights(*target);
		channel[&ch] = true;
	}
	return GOOD;
}

