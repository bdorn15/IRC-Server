#pragma once

#include "Global.hpp"

class Server;
class Client;

class Channel
{
	public:
		~Channel() = default;
		Channel(const std::string& name, Client* founder);

		Channel() = delete;
		Channel(const Channel& other) = delete;
		Channel& operator=(const Channel& rhs) = delete;

		//getters
		const std::string&			getChannelName() const;
		const std::string&			getChannelTopic() const;
		const std::string&			getChannelKey() const;
		const std::vector<Client*>	getChannelClients() const;
		const std::string&			getChannelModes() const;
		unsigned int				getChannelUserLimit() const;
		unsigned int				getChannelUserCount() const;
		time_t						getChannelCreationTime() const;
		std::pair<std::string, std::time_t> getChannelTopicInfo() const;

		//return a modifiable list of comma-space separated users/invites
		std::string	getChannelUserList() const;
		std::string	getChannelInviteList() const;

		//modifiers
		void setChannelTopic(const std::string& topic, const std::string& nick,
				const std::string& username, const std::string& host);
		void setChannelKey(const std::string& key);
		void setChannelUserLimit(unsigned int new_limit);

		void addChannelClient(Client* cl);
		void addChannelInvite(const std::string& nick);
		void addChannelMode(const char mode);
		void addChannelOperatorRights(Client& cl);

		void removeChannelMode(const char mode);
		void removeChannelUser(Client* cl);
		void removeChannelOperatorRights(const std::string& nick);

		std::string	buildChannelArgs() const; //needed for mode responses
		
		bool isChannelUser(const std::string& nick) const;
		bool isChannelOperator(const std::string& nick) const;
		bool isChannelInvite(const std::string& nick) const;
		bool isChannelMode(const char mode) const;
		bool isChannelKey(const std::string& key) const;

	private:
		std::string 				channelName; 
		std::string					channelTopic;
		std::string					channelKey;
		std::vector<Client*>		channelClients; //it would make sense to store this as map<nick, Client*>
		std::vector<std::string>	channelOperators;
		std::vector<std::string> 	channelInvites;
		std::string					channelModes;
		unsigned int				channelUserLimit;
		unsigned int				channelUserCount; 

		std::time_t					channelCreationTime;
		std::pair<std::string, std::time_t> channelTopicInfo;
};

