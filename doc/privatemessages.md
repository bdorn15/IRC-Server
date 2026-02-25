## Send and receive private messages

This is a client-to-client as well as client-to-channel command.
All private messages go through the server, there is no direct client connection.
All in channel communication goes from sending client via the server to other clients in the channel. This manifests as PRIVMSG to each receiving client.

Messages sent to a channel you are banned in will silently fail.
Messages sent by a server to a target beginning with  $  will be broadcast to all clients.


- Command: `PRIVMSG <target> :<message to be sent>`
	- OR: `PRIVMSG <comma sep target list> :<message to be sent>`
- Message example: `:source PRIVMSG target :message`
	- `<source>` can be either a nick or an address
	- `<target>` can be either a client or channel
		- Messages sent to a Channel are inherently PRIVMSG with the target simply #channelname.
		- Prefixing #channelname with channel member prefixes will only send the message to users with those prefixes.
			- These are mentioned in the [Join and Part channel](join-part_channels.md) page as part of the RPL_NAMREPLY (353).
- Server replies:-
	- ERR_NOSUCHNICK (401)
		- "`<client> <nickname>` :No such nick/channel"
	- ERR_NOSUCHSERVER (402)
		- "`<client> <server name>` :No such server"
	- ERR_CANNOTSENDTOCHAN (404)
		- "`<client> <channel>` :Cannot send to channel"
		- Can be a result of sending a message in a moderated (+m) channel without voice or better
		- Can be a result of sending a message to a channel you are not a member of that does not allow external messages (+n).
	- ERR_TOOMANYTARGETS (407)
		- "`<target> :<error code>` recipients. `<abort message>`"
		- If sending to a user@host with multiple instances.
		- If sending to too many targets.
	- ERR_NORECIPIENT (411)
		- "`<client>` :No recipient given (`<command>`)"
	- ERR_NOTEXTTOSEND (412)
		- "`<client>` :No text to send"
	- ERR_NOTOPLEVEL (413)
		- "`<mask>` :No toplevel domain specified"
	- ERR_WILDTOPLEVEL (414)
		- "`<mask> `:Wildcard in toplevel domain"
	- RPL_AWAY (301)
		- "`<client> <nick>` :`<message>`
		- If the target is set away, `<message>` is their away message.
			- (Maybe not for us, we are not required to implement away)
