## Client authentication

[About clients](clients.md)

The server listens for incoming connections and once one happens the connecting client must immediately attempt registration.

- Until registered not all commands are available.
	- Probably only the required registration commands allowed.
- Recommended command order (for us)
	- Capability negotiation is optional: https://ircv3.net/specs/extensions/capability-negotiation.html (probably not for us)
	- PASS
		- Client password must match the password given as server parameter.
		- Command: `PASS <password>`
		- Server replies:-
			- ERR_NEEDMOREPARAMS (461)
				- "`<client>` `<command>` :Not enough parameters"
			- ERR_ALREADYREGISTERED (462)
				- "`<client>` :You may not re-register"
					- When a client tries to change a detail that can only be set during registration (eg. re-sending PASS or USER after registration).
			- ERR_PASSWDMISMATCH (464)
				- "`<client>` :Password incorrect"

	- NICK
		- Command: `NICK <nickname>`
		- Server replies:-
			- ERR_NONICKNAMEGIVEN (431)
				- "`<client>` :No nickname given"
			- ERR_ERRONEUSNICKNAME (432)
				- "`<client>` `<nick>` :Erroneous nickname"
					- When a nick has illegal characters or otherwise breaks server nick guidelines.
			- ERR_NICKNAMEINUSE (433)
				- "`<client>` `<nick>` :Nickname is already in use"
			- ERR_NICKCOLLISION (436)
				- "`<client>` `<nick>` :Nickname collision KILL from `<user>@<host>`"
					- When a nick is already registered by another server.
			
			- Server can also send a NICK command to a user to confirm the client nick command worked. (Not for us?)

		- Nickname limits:-
			- Not allowed in nick: space , * ? ! @
			- Not allowed at start of nick: $ : # &
			- Specific prefixes given in https://modern.ircdocs.horse/#channel-membership-prefixes and https://ircv3.net/specs/extensions/multi-prefix
			- If specified in [RPL_ISUPPORT (005)](RPL_ISUPPORT-(005)), a maximum length.
				- `NICKLEN=<integer>`

	- USER
		- Command: `USER <username> 0 * :<realname>`
			- Newer protocol.
				- 0 is actually a bitmask of desired user mode
				- Older protocol has hostname and servername instead of `0` and `*`.
			- `:` required as real names often have spaces
		- Server replies:-
			- ERR_NEEDMOREPARAMS (461)
			- ERR_ALREADYREGISTERED (462)
		- User limits:-
			- Length, specified in an RPL_ISUPPORT message (not for us?)

`PING` can be sent by the server with the expectation of a `PONG` from the client, but this is optional. Might be good for ending unresponsive clients. Our choice I think though.

Once the client has finished its required commands the server then MUST send the following messages to the client, in the following order:
1. RPL_WELCOME (001)
	- "`<client>` :Welcome to the `<networkname>` Network, `<nick>[!<user>@<host>`]"
	- First parameter of the RPL_WELCOME (001) message is the nickname assigned by the network to the client.
		- Regardless of any inappropriate NICK commands sent by client.
2. RPL_YOURHOST (002)
	- "`<client>` :Your host is `<servername>`, running version `<version>`"
3. RPL_CREATED (003)
	- "`<client>` :This server was created `<datetime>`"
4. RPL_MYINFO (004)
	- "`<client>` `<servername>` `<version>` `<available user modes><available channel modes> [<channel modes with a parameter>]`"
5. at least one [RPL_ISUPPORT (005)](RPL_ISUPPORT-(005).md) numeric to the client.
	1. The server MAY then send other numerics and messages. (not for us)
6. Server SHOULD respond as though the client sent the LUSERS command
	- Command: `LUSERS`
	- Server replies:-
		- RPL_LUSERCLIENT (251)
			- "`<client>` :There are `<u>` users and `<i>` invisible on `<s>` servers"
		- RPL_LUSEROP (252)
			- "`<client>` `<ops>` :operator(s) online"
				- (number of IRC operators (IRCOPS), these are users who have server admin rights essentially)
		- RPL_LUSERUNKNOWN (253)
			- "`<client>` `<connections>` :unknown connection(s)"
		- RPL_LUSERCHANNELS (254)
			- "`<client>` `<channels>` :channels formed"
		- RPL_LUSERME (255)
			- "`<client>` :I have `<c>` clients and `<s>` servers"
				- Clients and servers connected to this server. (servers not relevant to us)
		- RPL_LOCALUSERS (265)
			- "`<client>` [`<u> <m>`] :Current local users `<u>`, max `<m>`"
				- Current and total number of clients connected to this server.
		- RPL_GLOBALUSERS (266)
			- "`<client>` [`<u> <m>`] :Current global users `<u>`, max `<m>`"
				- Current and total number of clients connected to this network (direct and through other servers to us) (not for us)

7. Server MUST respond as though the client sent the MOTD command
	- Command: `MOTD`
	- Server replies:-
		- ERR_NOSUCHSERVER (402)
			- "`<client>` `<server name>` :No such server"
				- If client requests MOTD from a server that does not exist
		- ERR_NOMOTD (422)
			- "`<client>` :MOTD File is missing"
		- RPL_MOTDSTART (375)
			- "`<client>` :- `<server>` Message of the day - "
		- RPL_MOTD (372)
			- "`<client>` :`<contents of the motd>`"
				- Lines may be wrapped to 80 characters, terminal compatibility 
		- RPL_ENDOFMOTD (376)
			- "`<client>` :End of /MOTD command."

8. If the user has client modes set on them automatically upon joining the network, the server SHOULD send the client the RPL_UMODEIS (221) reply or a MODE message with the client as target, preferably the former.
	- RPL_UMODEIS (221)
		- "`<client> <user modes>`"
	- MODE
		- (Same command for user and channel, only user here)
		- Command: `MODE <target> [<modestring> [<mode arguments>...]`
			- On auth, only: `MODE <target>`
		- Server replies:-
			- To `<target>`
				- ERR_NOSUCHNICK (401)
					- "`<client> <nickname>` :No such nick/channel"
				- ERR_USERSDONTMATCH (502)
					- "`<client>` :Cant change mode for other users"
			- To `<modestring>`
				- No `<modestring>`: RPL_UMODEIS (221)
					- Return current usermodes to client.
				- ERR_UMODEUNKNOWNFLAG (501)
					- "`<client>` :Unknown MODE flag"
					- If one of the modes is not supported by the server
