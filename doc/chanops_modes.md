## Channel operators and their commands

In the subject, only users and chanops are required.
Chanops have their nickname prefixed by @, in the channel names list.
(Other modes exist and are voice (+), half-op (%) and founder (~). Not relevant for us)

[About channels.](channels.md)

To make a client an op, an existing operator uses the /MODE command, see below.

Chanops have commands only they can perform while within the channel they are operators on. These are:-
- KICK
	- Removes a client from the channel
		- The client can rejoin immediately, (however we are not required to implement a ban list, so....)
	- If specified in [RPL_ISUPPORT (005)](RPL_ISUPPORT-(005)), the comment given for a kick can be of restricted length.
		- `KICKLEN=<integer>`
	- Command: `KICK <channel> <user> <comment>`
	- Message: `:source KICK #channel user`
	- Server replies:- 
		- ERR_NEEDMOREPARAMS (461)
			- "`<client>` `<command>` :Not enough parameters"
		- ERR_NOSUCHCHANNEL (403)
			- "`<client> <channel>` :No such channel"
		- ERR_CHANOPRIVSNEEDED (482)
			- "`<client> <channel>` :You're not channel operator"
		- ERR_USERNOTINCHANNEL (441)
			- "`<client> <nick> <channel>` :They aren't on that channel"
		- ERR_NOTONCHANNEL (442)
			- "`<client> <channel>` :You're not on that channel"
		- ERR_BADCHANMASK (476)
			- "`<client> <channel>` :Bad Channel Mask"
			- This is sent when the channel name contains illegal characters

- INVITE
	- Invite clients to join an invite only channel (+i)
	- Command: `INVITE <nickname> <channel>`
	- Message: `:source INVITE user #channel`
	- Server replies:- 
		- RPL_INVITING (341)
			- "`<client> <nick> <channel>`"
		- ERR_NEEDMOREPARAMS (461)
		- ERR_NOSUCHCHANNEL (403)
		- ERR_NOTONCHANNEL (442)
		- ERR_CHANOPRIVSNEEDED (482)
		- ERR_USERONCHANNEL (443)
			- "`<client> <nick> <channel>` :is already on channel"

- TOPIC
	- Sets the topic of the channel. Only ops can set topic if channel mode is (+t)
	- Command: `TOPIC <channel> <topic>`
		- OR to read the topic: `TOPIC <channel>`
	- Message: `TOPIC #channel :topic`
		- OR to read the topic: `TOPIC #channel
	- Server replies:- 
		- ERR_NEEDMOREPARAMS (461)
		- ERR_NOSUCHCHANNEL (403)
		- ERR_NOTONCHANNEL (442)
		- ERR_CHANOPRIVSNEEDED (482)
		- RPL_NOTOPIC (331)
			- "`<client> <channel>` :No topic is set"
		- RPL_TOPIC (332)
			- "`<client> <channel> :<topic>`"
		- RPL_TOPICWHOTIME (333)
			- "`<client> <channel> <nick> <setat>`"

- MODE
	- Set the channel mode:-
		- i, t
			- invite only, restricted topic
		- k `<key>`, l `<number>`
			- channel key, user limit
		- o
			- Sets a user as a channel operator
	- (Same command for user and channel, only channel here)
	- Command: `MODE <target> [<modestring> [<mode arguments>...]]`
		- `<modestring>` consists of a string of + or - followed by mode letters and parameters. + adds a mode, - removes a mode.
		- `MODE <target>` - requests modes of target.
	- Message: `:source MODE #channel -it+kl test,10`
				- Add: channel key (test), user limit (10)
				- Remove: invite only, restricted topic
			- `:source MODE #channel +o user` - ops user
	- Server replies:-
		- ERR_CHANOPRIVSNEEDED (482)
		- To `<target>`
			- ERR_NOSUCHCHANNEL (403)
		- To `<modestring>`
			- No `<modestring>`: RPL_CHANNELMODEIS (324)
				- "`<client> <channel> <modestring> <mode arguments>..`"
					- Informs client of channel modes.
					- Good idea to hide channel keys and other sensitive information.
				- RPL_CREATIONTIME (329)
					- "`<client> <channel> <creationtime>`"
					- Should be sent after RPL_CHANNELMODEIS (324).
			- ERR_UMODEUNKNOWNFLAG (501)
				- "`<client>` :Unknown MODE flag"
				- If one of the modes is not supported by the server
		- When setting +k
			- ERR_INVALIDMODEPARAM (696)
				- "`<client> <target chan/user> <mode char> <parameter> :<description>`"
				- This is not as standard as 525, we can probably not worry about this.
			- ERR_INVALIDKEY (525)
				- "`<client> <target chan>` :Key is not well-formed"
