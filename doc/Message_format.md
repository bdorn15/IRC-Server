## Message string format

While a client is connected to a server, they send a stream of bytes to each other.
This stream contains messages.
These messages are the entirety of server/client communication.

- Messages are encoded in [UTF-8](https://en.wikipedia.org/wiki/UTF-8).
- Messages are no longer then 512 bytes long.
	- Messages longer than this limit can be handled in one of three ways:
		- Send ERR_INPUTTOOLONG (417)
			- "`<client>` :Input line was too long"
			- (We should probably do this, it seems the safest and easiest.)
		- Truncated at the 510th byte and the CRLF added.
		- Ignore silently or close the connection.
- Messages are separated by CRLF characters (\r\n)
	- These must be sent at the end of every message
	- Read incoming messages into a buffer and only parse once the CRLF has been received.
		- Empty messages and lines should be silently ignored.
		- Any messages with an incomplete or missing separator should be silently ignored.

- Messages are formatted thus:
	- `@<tags> :<source>  <command> <parameters> <CRLF>`
		- Four segments separated by a *single* space.
		- `@<tags>` optional, we can probably ignore
			- https://ircv3.net/specs/extensions/message-tags.html
		- `:<source>` Source of the message
			- `<servername> OR <nickname>!<user>@<host>`
			- Not necessary for every message
			- "Clients MUST NOT include a source when sending a message."
		- `<command>`, mandatory, any command in the protocol
			- `<parameters>`, any parameters required by the command
				- The final parameter can be prefixed with a colon `:`
				- This allows for strings containing SPACEs, such as chat messages and so on.