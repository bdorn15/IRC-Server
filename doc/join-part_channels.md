## Joining and parting channels

The client has to request to join a channel.
If they are permitted to do so then the server adds them to the channel.
PART is not explicitly called for but it makes sense that it is needed if we have JOIN.

#### Join command
- Command: `JOIN <channel> <key>`
	- Or: `JOIN <comma sep channel list> <comma sep key list>`
- Message example: `:user JOIN #channel`
	- Or: `:address JOIN #channel`
- Server replies:-
	- ERR_NEEDMOREPARAMS (461)
		- "`<client>` `<command>` :Not enough parameters"
	- ERR_NOSUCHCHANNEL (403)
		- "`<client> <channel>` :No such channel"
	- ERR_TOOMANYCHANNELS (405)
		- "`<client> <channel>` :You have joined too many channels"
	- ERR_BADCHANNELKEY (475)
		- "`<client> <channel>` :Cannot join channel (+k)"
	- ERR_BANNEDFROMCHAN (474)
		- "`<client> <channel>` :Cannot join channel (+b)"
	- ERR_CHANNELISFULL (471)
		- "`<client> <channel>` :Cannot join channel (+l)"
	- ERR_INVITEONLYCHAN (473)
		- "`<client> <channel>` :Cannot join channel (+i)"
	- ERR_BADCHANMASK (476)
		- "`<client> <channel>` :Bad Channel Mask"
		- This is sent when the channel name contains illegal characters
	- RPL_TOPIC (332)
		- "`<client> <channel>` :`<topic>`"
		- Returns the current channel topic.
	- RPL_TOPICWHOTIME (333)
		- "`<client> <channel> <nick> <setat>`"
		- Returns who and when the topic was set.
	- RPL_NAMREPLY (353)
		- "`<client> <symbol> <channel> :[prefix]<nick>{ [prefix]<nick>}`"
			- `<symbol>` =, @ or * depending on channel status
				- = = public channel (Probably only one relevant to us)
				- @ = secret channel
				- * = private channel
			- `[prefix]` client channel member prefix
				- ~, &, @, % and +
				- Likely only @ (operator) relevant to us.
				- Lack of a prefix indicates normal unprivileged user.
		- Same as if client had sent the **NAMES** command for the joined channel only
			- We need this reply, so we may as well have the NAMES command too.
	- RPL_ENDOFNAMES (366)
		- "`<client> <channel>` :End of /NAMES list"

#### Part command
- Command: `PART <channel> <reason>`
	- Or: `PART <comma sep channel list> <reason>`
- Message example: `:user PART #channel`
	- Or: `:address PART #channel`
- Server replies:-
	- ERR_NEEDMOREPARAMS (461)
	- ERR_NOSUCHCHANNEL (403)
	- ERR_NOTONCHANNEL (442)
		- "`<client> <channel>` :You're not on that channel"
- Server can send this message to a client to notify that client that a user has been removed from a channel.
	- `<source>` should be the client being removed
	- In this special case, only one channel should be in the message at a time.