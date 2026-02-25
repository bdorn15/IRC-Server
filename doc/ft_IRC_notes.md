## Subject requirements
#### Create an IRC server
- Standalone, only client connections
	- Multiple clients
	- Use a single reference client for evaluation
		- Has to work without errors
		- Hive computers have irssi at least, hexchat possibly?
- Usage: `./ircserv <port> <password>`
- Non-blocking IO
	- poll() with non-blocking file descriptors or similar
		- https://beej.us/guide/bgnet/html/split-wide/slightly-advanced-techniques.html#slightly-advanced-techniques
- TCP/IP connection
- Some protocol references
	- https://datatracker.ietf.org/doc/html/rfc1459
	- https://datatracker.ietf.org/doc/html/rfc2812
	- https://modern.ircdocs.horse/
	- https://ircv3.net/irc/

 - [Communication and message formatting](Message_format.md).

#### Functional requirements
	Quote: "you only have to implement the following features:"

- Clients must be able to
	- [Authenticate](Authenticate.md)
		- Set nick
		- Set username
	- [Join channels](join-part_channels.md) (and by extension part channels)
	- [Send and receive private messages](privatemessages.md)
- In channel messages must be shared to all clients in the channel
	- [About channels](channels.md)
- Channels must have users and [operators](chanops_modes.md)
	- (No mention of voice (+), half-op (%) or founder (~) users)
- Channel [operators must have the following commands](chanops_modes.md)
	- Kick
	- Invite
	- Topic
	- Mode
		- Modes: i, t, k, o, l

#### Additional not in subject
We are not asked for these, but they may be good for quality of life.

- PING, PONG
	- Can help to distinguish unresponsive or broken connections.
- QUIT
	- A clean way for the client to terminate the connection.
- KILL
	- A clean way for the server to terminate a client connection.
- RESTART
	- Force the server to restart.
- EXIT
	- Not a protocol standard, but it would be good to have a way to cleanly close all connections, free memory and exit the server.

#### Misc
Parser tester for IRC messages: https://github.com/ircdocs/parser-tests