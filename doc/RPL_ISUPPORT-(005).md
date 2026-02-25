## RPL_ISUPPORT message details

This numeric message tells the client what features and limits the server has.

(Uncertain whether we need this.)

- Things like:
	- How the server handles attributes (nicks, users) with different cases of character. (Case sensitivity)
	- How many channels a client can join.
	- The maximum length of nicks and channel names.
	- Among many others.

- RFC1459 (original rfc) does not support 005, so maybe we do not need to worry about it. Although this may depend on our chosen client. New protocol does use terms like "MUST send"
	- https://stackoverflow.com/questions/11585654/do-all-irc-servers-send-005-rpl-isupport

- Sources:
	- https://modern.ircdocs.horse/#rplisupport-005
		- https://modern.ircdocs.horse/#rplisupport-parameters
		- https://www.ircnet.com/articles/rpl_isupport
		- https://defs.ircdocs.horse/defs/isupport.html

The message to the client is formatted as such:
- `"<client> <1-13 tokens>` :are supported by this server"
	- tokens are in the format `parameter` or `parameter=value`
	- `-parameter` is used to remove an existing parameter