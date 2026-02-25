## About channels

On joining a non-existent channel the channel is created and the first joining client is automatically set as op.

Once the last client leaves a channel, the channel is deleted.

- Channel names.
	- Begin with # (There are others, but these are not for us.)
	- Can not contain SPACEs, commas or \a (system bell)
	- If specified in [RPL_ISUPPORT (005)](RPL_ISUPPORT-(005)), a maximum length.
		- `CHANNELLEN=<integer>`

- Channel topic.
	- Is sent to every client who joins the channel.
	- Has a hard limit of the maximum length of a message minus all non topic string information.
	- If specified in [RPL_ISUPPORT (005)](RPL_ISUPPORT-(005)), a smaller maximum length.
		- `TOPICLEN=<integer>`

- Channel modes ([more depth](https://www.unrealircd.org/docs/Channel_modes)).
	- Defaults are up to the server to decide.
		- Usually +t and +n
	- Are indicated by a single character.
	- Some take a parameter.
	- For command and message details [see here](chanops_modes.md).
	- If specified in [RPL_ISUPPORT (005)](RPL_ISUPPORT-(005)), the allowed modes can be limited.
		- `CHANMODES=<comma sep string of mode letters>`
	- We only need to concern ourselves with:
		- i
			- The channel can only be joined by a client who has recieved a valid INVITE.
		- t
			- Channel topic can only be changed by channel operators.
		- k `<key>`
			- Channel joining is limited by clients who provide the correct `<key>`.
		- l `<number>`
			- Channel can only host `<number>` of clients.
		- o
			- Not a channel mode, but a user mode only channel operators can set. Sets a user as a channel operator.