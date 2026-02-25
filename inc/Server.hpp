#pragma once

#include "Global.hpp"

class Client;
class Channel;

class Server
{
	private:

		int 							_port;
		std::string 					_password;
		std::map<int, Client*> 			_clients;
		std::map<std::string, Channel*> _channels;
		int 							_serverSocket;
		bool 							_running; 		//for main loop

		//All the following
		//Moved over from class Client during refactoring.
		//ClientValidityChecks.cpp
		bool isNickValid_(const std::string &newNick);
		bool isNickAvailable_(const std::string &newNick);
		bool isUsernameValid_(const std::string &username);

		//ReplyMessages.cpp
		int confirmReg_(Client &clientObj);
		//End of client function move.
		//Remove all these comments once we are all familiar with the changes.

		//ChannelUserInfo.cpp
		std::string fetchAllNicks(void);

		
	public:

		Server() = delete;
		~Server();
		Server(const Server &other) = delete;
		Server& operator=(const Server &other) = delete;

		Server(int port, const std::string &password);

		//getters and setters
		int getServerSocket() const;	//not in use
		bool getState(); 				//not in use yet
		void setClient(int, Client*);
		void setChannel(Channel*);
		Client* getClient(int);
		
		//member functions
		void run();
		void stop();
		void initServerSocket();
		void removeClient(int);
		void removeChannel(const std::string& ch_name);

		//The next two functions are needed for commands
		Channel* findChannel(const std::string& ch_name) const;
		Client* findClient(const std::string& cl_nick) const;

		//ReplyMessages.cpp
		std::string buildMsgHeader(const int &numeric, Client &clientObj);
		std::string buildMessage(const int &numeric, Client &clientObj, const std::string &arg1, const std::string &arg2, const std::string &arg3);
		void doMessage(const int &numeric, Client &clientObj, const std::string &arg1, const std::string &arg2, const std::string &arg3);
		int sendMessage(Client &clientObj, const std::string &message);

		//IRC Commands
		//Registration.cpp
		int PASS(Client &clientObj, const std::vector<std::string> &cmdVec);
		int USER(Client &clientObj, const std::vector<std::string> &cmdVec);
		void MOTD(Client &clientObj);

		//NICK.cpp
		int NICK(Client &clientObj, const std::vector<std::string> &cmdVec);

		//int MODE(Client &clientObj, const std::vector<std::string> &cmdVec, const std::string &longMsg);
		//From Channel merge:
		void PART(Client& cl, const std::vector<std::string>& par);
		void TOPIC(Client& cl, const std::vector<std::string>& par);
		void INVITE(Client& cl, const std::vector<std::string>& par);
		void KICK(Client& cl, const std::vector<std::string>&par);
		void MODE(Client& cl, const std::vector<std::string>&par);
		
		//JOIN.cpp
		int JOIN(Client &clientObj, const std::vector<std::string> &cmdVec);
		//QUIT.cpp
		void QUIT(Client &clientObj, const std::vector<std::string> &cmdVec);
		
		//ChannelUserInfo.cpp
		void NAMES(Client &clientObj, const std::vector<std::string> &cmdVec);
		void WHO(Client &clientObj, const std::vector<std::string> &cmdVec);
		//For testing
		void LUSERS(Client &clientObj);
		void WHOIS(Client &clientObj, const std::vector<std::string> &cmdVec);
		void LIST(Client &clientObj);

		//WiffWaff.cpp
		int PING(Client &clientObj, const std::vector<std::string> &cmdVec);
		void PONG(Client &clientObj, const std::vector<std::string> &cmdVec);

		void PRIVMSG(std::vector<std::string> args, std::string message, int senderFD);
};
