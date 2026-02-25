#pragma once

//CPP includes
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream> 
#include <map>
#include <unordered_map>
#include <vector>
#include <set>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <cctype>
#include <ctime>
#include <optional>
#include <stdexcept>

//C includes and linux system calls
#include <sys/socket.h>	 // socket, bind, listen //send() libc 
#include <arpa/inet.h>  // sockaddr_in, htons
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <poll.h>

//Project includes
#include "Server.hpp"
#include "ValidArgs.hpp"
#include "Client.hpp"
#include "Channel.hpp"

/**
 * @brief ft_IRC program related values.
 */
typedef enum e_program_settings
{
	CLIENT_CMD_CNT	= 3,
	CLIENT_CONF_CNT	= 4
} t_program_settings;

/**
 * @brief IRC reply (RPL) messages.
 * @note nick and client are different. Nick is just the nick. client is the nick,
 * user, host combination.
 */
#define MSG_HEADER(servername, msgnum, nick) (":" + servername + " " + msgnum + " " + nick + " ") //First part of all messages
#define RPL_WELCOME(servername, client) (":Welcome to the " + servername + " Network " + client + "\r\n") //001
#define RPL_YOURHOST(servername, version) (":Your host is " + servername + ", running version " + version + "\r\n") //002
#define RPL_CREATED(date) (":This server was created " + date + "\r\n") //003
#define RPL_MYINFO(servername, version, usermodes, chanmodes) (servername + " " + version + " " + usermodes + " " + chanmodes + "\r\n") //004
#define RPL_ISUPPORT(tokens) (tokens + " :are supported by this server\r\n") //005
#define RPL_LUSERCLIENT(users, invisible, servers) (":There are " + users + " users and " + invisible + " invisible on " + servers + " servers\r\n") //251
#define RPL_LUSERCHANNELS(channels) (channels + " :channels formed\r\n") //254
#define RPL_AWAY(nick, message) (nick + " :" + message + "\r\n") //301
#define RPL_WHOISSERVER(nick, server, serverinfo) (nick + " " + server + " :" + serverinfo + "\r\n") //312
#define RPL_ENDOFWHO(mask) (mask + " :End of /WHO list\r\n") //315
#define RPL_ENDOFWHOIS(nick) (nick + " :End of /WHOIS list\r\n") //318
#define RPL_WHOISCHANNELS(nick, channellist) (nick + " :" + channellist + "\r\n") //319
#define RPL_LISTSTART "Channel :Users Name\r\n" //321
#define RPL_LIST(channel, clients, topic) (channel + " " + clients + " " + topic + "\r\n") //322 
#define RPL_LISTEND ":End of /LIST\r\n" //323
#define RPL_CHANNELMODEIS(channel, modes, modeargs) (channel + " " + modes + " " + modeargs + "\r\n") //324
#define RPL_CREATIONTIME(channel, time) (channel + " " + time + "\r\n") //329
#define RPL_NOTOPIC(channel) (channel + " :No topic is set\r\n") //331
#define RPL_TOPIC(channel, topic) (channel + " " + topic + "\r\n") //332
#define RPL_TOPICWHOTIME(channel, setter, time) (channel + " " + setter + " :" + time + "\r\n") //333
#define RPL_INVITE(channel, target) (target + " " + channel + "\r\n") //341
#define RPL_NAMREPLY(channel, symbol, nicklist) (symbol + " " + channel + " :" + nicklist + "\r\n") //353
#define RPL_ENDOFNAMES(channel) (channel + " :End of /NAMES list\r\n") //366
#define RPL_MOTD(line) (line + "\r\n") //372
#define RPL_MOTDSTART(server) ("- " + server + " Message of the Day -\r\n") //375
#define RPL_ENDOFMOTD "End of /MOTD command.\r\n" //376
//Special cases
//The "H :0" refers to the away status of the user and the number of server hops
//between the user doing the /WHO and the target. These are set to defaults for us.
#define RPL_WHOREPLY(channel, username, host, server, nick, flags, realname) (channel + " " + username + " " + host + " " + server + " " + nick + " H" + flags + " :0 " + realname + "\r\n") //352
#define RPL_WHOISUSER(nick, username, host, realname) (nick + " " + username + " " + host + " * " + realname + "\r\n") //311
																						 
/**
 * @brief special messages that don't have default header
 */
#define MSG_TOPIC(channel, topic) (" TOPIC " + channel + " " + topic + "\r\n") //topic message to all clients in the channel
#define MSG_INVITE(target, channel) (" INVITE " + target + " :" + channel + "\r\n") //invitation message to target
#define MSG_KICK(channel, target, comment) (" KICK " + channel + " " + target + " " + comment + "\r\n") //kick message to all clients in channel and the target itself
#define MSG_PART(channel, reason) (" PART " + channel + " " + reason + "\r\n") //part msg to all clients in channel and the nick itself
#define MSG_MODE(channel, modes, args) (" MODE " + channel + " " + modes + " " + args + "\r\n") //mode msg to all clients in channel
#define MSG_QUIT(reason) (" QUIT :Quit: " + reason + "\r\n") //Quit message, plus reason if present.

/**
 * @brief Error codes.
 * @details Error 666 is not standard and serves as our own error.
 */
typedef enum e_err_code
{
	GOOD					= 0,
	BAD						= 666,
} t_err_code;

/**
 * @brief IRC error (ERR) messages.
 */
#define ERR_UNKNOWNERROR(command, subcommand, info) (command + " " + subcommand + " :" + info + "\r\n") //400
#define ERR_NOSUCHNICK(nickname) (nickname + " :No such nick/channel\r\n") //401
#define ERR_NOSUCHSERVER(servername) (servername + " :No such server\r\n") //402
#define ERR_NOSUCHCHANNEL(channel) (channel + " :No such channel\r\n") //403
#define ERR_CANNOTSENDTOCHAN(channel) (channel + " :Cannot send to channel\r\n") //404
#define ERR_TOOMANYCHANNELS(channel) (channel + " :You have joined too many channels\r\n") //405
#define ERR_NOORIGIN " :No origin specified\r\n" //409
#define ERR_NORECIPIENT(command) (" :No recipient given(" + command + ")\r\n") //411
#define ERR_NOTEXTTOSEND " :No text to send\r\n" //412
#define ERR_INPUTTOOLONG " :Input line was too long\r\n" //417
#define ERR_UNKNOWNCOMMAND(command) (command + " :Unknown command\r\n") //421
#define ERR_NOMOTD " :MOTD File is missing" //422
#define ERR_NONICKNAMEGIVEN " :No nickname given\r\n" //431
#define ERR_ERRONEUSNICKNAME(nick) (nick + " :Erroneous nickname\r\n") //432
#define ERR_NICKNAMEINUSE(nick) (nick + " :Nickname is already in use\r\n") //433
#define ERR_USERNOTINCHANNEL(nick, channel) (nick + " " + channel + " :They aren't on that channel\r\n") //441
#define ERR_NOTONCHANNEL(channel) (channel + " :You're not on that channel\r\n") //442
#define ERR_USERONCHANNEL(nick, channel) (nick + " " + channel + " :is already on channel\r\n")  //443
#define ERR_NOTREGISTERED ":You have not registered\r\n" //451
#define ERR_NEEDMOREPARAMS(command) (command + " :Not enough parameters\r\n") //461
#define ERR_ALREADYREGISTERED " :You may not reregister\r\n" //462
#define ERR_PASSWDMISMATCH " :Password incorrect\r\n" //464
#define ERR_CHANNELISFULL(channel) (channel + " :Cannot join channel (+l)\r\n") //471
#define ERR_UNKNOWNMODE(nick, server, mode) (":" + server + " 472 " + nick + " " + mode + " :is an unknown mode char to me\r\n") //472
#define ERR_INVITEONLYCHAN(channel) (channel + " :Cannot join channel (+i)\r\n") //473
#define ERR_BANNEDFROMCHAN(channel) (channel + " :Cannot join channel (+b)\r\n") //474
#define ERR_BADCHANNELKEY(channel) (channel + " :Cannot join channel (+k)\r\n") //475
#define ERR_BADCHANMASK(channel) (channel + " :Bad Channel Mask\r\n") //476
#define ERR_CHANOPRIVSNEEDED(channel) (channel + " :You're not channel operator\r\n") //482
#define ERR_UMODEUNKNOWNFLAG " :Unknown MODE flag\r\n" //501
#define ERR_USERSDONTMATCH " :Can't change mode for other users\r\n" //502

/**
 * @brief IRC protocol related values.
 */
typedef enum e_protocol_settings
{
	MSG_LEN_MAX				= 512
} t_protocol_settings;

/**
 * @brief IRC protocol related defines.
 */
#define MSG_DELIM "\r\n"

/**
 * @brief IRC server related values.
 * @details X_LEN_MAX: chars, REG_TIMEOUT: seconds
 */
typedef enum e_server_settings
{
	NICK_LEN_MAX			= 30,
	USER_LEN_MAX			= 30,
	CHAN_LEN_MAX			= 30,
	REG_TIMEOUT				= 120
} t_server_settings;

/**
 * @brief IRC server related defines.
 */
#define CHANNEL_MODES "klit"
#define ISUPPORT_CHANMODES ",,kl,it"
#define USER_MODES ""
#define UNAME_ILLEGAL_CHARS "#&@ :,."
#define NICK_ILLEGAL_CHARS "#&@ :,."
#define CHAN_ILLEGAL_CHARS " \a:,."

/**
 * @brief ft_IRC program related defines.
 */
#define NETWORK_NAME "ft_irc"
#define SERVER_NAME "NANA"
#define SERVER_VERSION "NANA v0.3.2"
#define SERVER_INFO "42 School ft_irc project."
#define MOTD_FILE "./inf/motd.txt"

//Hashing function stolen from stackoverflow
//HandleClientMessages.cpp
constexpr unsigned int hash(const char *s, int off = 0);

//FreeHelpers.cpp
std::vector<std::string> xsvSplit(const std::string string, const char delim);
bool isInXSV(const std::string needle, const std::string string, const char delim);
