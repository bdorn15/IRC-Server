#include "Channel.hpp"

/*==============================*/
/*		CONSTRUCTOR				*/
/*==============================*/
Channel::Channel(const std::string& name, Client* founder)
	: channelName(name), channelTopic(""), channelModes("t"), channelUserLimit(0),
	channelUserCount(1), channelCreationTime(std::time(nullptr))
{
	/* TODO
	 * BUILD A DEFAULT TOPIC
	 * Channel topics commonly state channel rules, links, quotes from channel members,
	 * a general description of the channel, or whatever the channel operators want to share with the clients in their channel.
	 */
	channelClients.emplace_back(founder);
	founder->addChannel(this, true);
	channelOperators.emplace_back(founder->getNick());
}

/*==============================*/
/*			GETTERS				*/
/*==============================*/
const std::string& Channel::getChannelName(void) const
{
	return channelName;
}

const std::string& Channel::getChannelTopic() const
{
		return channelTopic;
}

const std::string& Channel::getChannelKey() const
{
	return channelKey;
}

const std::vector<Client*> Channel::getChannelClients() const
{
	return channelClients;
}

std::string Channel::getChannelUserList() const
{
	std::string ret;
	auto it = channelClients.begin();
	auto ite = channelClients.end();

	if (it == ite)
		return "";

	for (; it != ite; it++) {
		Client* cl = *it;
		const std::string nick = cl->getNick();
		if (isChannelOperator(nick))
			ret += "@";
		ret += nick + " ";
	}
	ret.pop_back(); //remove the extra space at the end
	return ret;
}

std::string Channel::getChannelInviteList() const
{
	std::string ret;

	if (channelInvites.empty())
		return "";

	for (const std::string& nick : channelInvites) {
		ret += nick + ", ";
	}
	ret.erase(ret.size() - 2); //remove the extra ", " at the end
	return ret;
}

const std::string& Channel::getChannelModes() const
{
	return channelModes;
}

unsigned int Channel::getChannelUserLimit() const
{
	return channelUserLimit;
}

unsigned int Channel::getChannelUserCount() const
{
	return channelUserCount;
}

std::pair<std::string, std::time_t> Channel::getChannelTopicInfo() const
{
	return channelTopicInfo;
}

std::time_t Channel::getChannelCreationTime() const
{
	return channelCreationTime;
}

/*==============================*/
/*			SETTERS				*/
/*==============================*/
void Channel::setChannelTopic(const std::string& topic, const std::string& nick,
		const std::string& username, const std::string& host)
{
	this->channelTopic = topic;
	std::string setter = nick + "!~" + username + "@" + host;
	channelTopicInfo.first = setter;
	channelTopicInfo.second = std::time(nullptr);
}

void Channel::setChannelKey(const std::string& key)
{
	this->channelKey = key;
}

void Channel::setChannelUserLimit(unsigned int new_limit)
{
	this->channelUserLimit = new_limit;
}

/*==============================*/
/*			ADDERS				*/
/*==============================*/
void Channel::addChannelClient(Client* cl)
{
	std::string& nick = cl->getNick();
	if (isChannelInvite(nick)) {
		auto it = std::find(channelInvites.begin(), channelInvites.end(), nick);
		channelInvites.erase(it);
	}
	channelClients.push_back(cl);
	cl->addChannel(this, false);
	channelUserCount++;
}

void Channel::addChannelInvite(const std::string& nick)
{
	if (isChannelInvite(nick))
		return ;
	channelInvites.push_back(nick);
}

void Channel::addChannelMode(const char new_mode)
{
	if (isChannelMode(new_mode))
		return ;
	channelModes += new_mode;
}

void Channel::addChannelOperatorRights(Client& cl)
{
	std::string nick = cl.getNick();
	if (not isChannelUser(nick))
		return ;

	channelOperators.push_back(nick);
	std::map<Channel*, bool>& channels = cl.getChannelMap();
	channels[this] = true;
}

/*==============================*/
/*			REMOVERS			*/
/*==============================*/
void Channel::removeChannelMode(const char rm_mode)
{
	if (isChannelMode(rm_mode)) {
		if (rm_mode == 'k') {
			channelKey.clear();
		}
		else if (rm_mode == 'l') {
			channelUserLimit = 0;
		}
		std::string::size_type pos = channelModes.find_first_of(rm_mode);
		channelModes.erase(pos, 1);
	}
}

void Channel::removeChannelUser(Client* toRemove)
{
	auto it = std::find(channelClients.begin(), channelClients.end(), toRemove);
	if (it != channelClients.end()) {
		removeChannelOperatorRights(toRemove->getNick());
		channelClients.erase(it);
		channelUserCount--;
	}
}

void Channel::removeChannelOperatorRights(const std::string& nick)
{
	auto it = std::find(channelOperators.begin(), channelOperators.end(), nick);
	if (it != channelOperators.end()){
		channelOperators.erase(it);
	}
}

/*==============================*/
/*			OTHERS				*/
/*==============================*/
std::string Channel::buildChannelArgs(void) const
{
	std::string args;

	for (size_t i = 0; channelModes[i]; i++) {
		if (channelModes[i] == 'k') {
			args += channelKey;
			args += " ";
		}
		else if (channelModes[i] == 'l') {
			args += std::to_string(channelUserLimit);
			args += " ";
		}
	}
	if (not args.empty())
		args.pop_back();
	return args;
}

/*==============================*/
/*			HELPERS				*/
/*==============================*/
bool Channel::isChannelUser(const std::string& nick) const
{
	for (Client* client : channelClients) {
		if (client->getNick() == nick)
			return true;
	}
	return false;
}

bool Channel::isChannelOperator(const std::string& nick) const
{
	auto it = std::find(channelOperators.begin(), channelOperators.end(), nick);
	if (it != channelOperators.end())
		return true;
	return false;
}

bool Channel::isChannelInvite(const std::string& nick) const
{
	for (auto invite : channelInvites) {
		if (invite == nick)
			return true;
	} 
	return false;
}

bool Channel::isChannelMode(const char mode) const
{
	std::string::size_type pos = channelModes.find_first_of(mode);

	if (pos != std::string::npos)
		return true;
	return false;
}

bool Channel::isChannelKey(const std::string& key) const
{
	if (isChannelMode('k'))
		return channelKey == key;
	return true;
}
