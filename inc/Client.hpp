#pragma once

#include "Global.hpp"

struct t_registration
{
	//Registration information and processing
	std::vector<std::string> passString;
	std::vector<std::string> nickString;
	std::vector<std::string> userString;
	time_t regStart;
	unsigned char regBufferOK = 0;
	bool clientPassOK = false;
	bool clientNickOK = false;
	bool clientNickFailed = false;
	bool clientRegOK = false;
};

class Channel;
class Server;

class Client
{
private:
	int clientFD_;
	std::string inBuffer_;
	t_registration clientReg_;

	std::string clientNick_;
	std::string clientUsername_;
	std::string clientRealname_;
	std::string clientHost_;

	//Channel object, operator status (channel membership prefix: o)
	//https://modern.ircdocs.horse/#channel-membership-prefixes
	std::map<Channel*, bool>	clientChannels_;

	//HandleClientMessages.cpp
	std::vector<std::string> bufferSplitMessages_(void);
	int checkMessageValidity_(Client &clientObj, Server &serverObj, const std::string &tmpMessage);
	std::vector<std::string> splitMessage_(const std::string &clientMessage);
	int executeMessage_(Client &clientObj, Server &serverObj, std::vector<std::string> &tmpVec);

	//Registration.cpp
	int doRegistration_(Server &serverObj, std::vector<std::string> &cmdVec);

public:
	//Default destructor
	~Client() {};

	//Constructor
	Client(int clientFD, const std::string& host);
	
	//Getters
	int getFd(void) const;
	std::string &getInBuffer(void);
	std::string &getNick(void);
	std::string &getUsername(void);
	std::string &getRealname(void);
	std::string &getHost(void);
	std::string getPrefix(void);
	std::map<Channel*, bool> &getChannelMap(void);
	std::string getChannels(void);
	time_t getConTime(void) const;
	std::string getUserParam(void) const;
	bool isOpOfChan(Channel *channel);
	bool isRegistered(void);
	bool hasSetNick(void);
	bool hasPassBeenGiven(void);

	//HandleClientMessages.cpp
	bool processBuffer(Server &serverObj);
	
	//Setters
	void setRegistration(bool regValue);
	void setPassGiven(bool passValue);
	void setNickStatus(bool nickValue);
	void setNickFailStatus(bool nickValue);

	//Channel list methods
	void addChannel(Channel *channelObj, bool operatorStatus);
	void removeChannel(Channel *channelObj);
};
