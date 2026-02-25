## About clients

In addition to a unique nickname. Servers must store the real name/address of the host that the client is connecting from, the username of the client on that host, and the server to which the client is connected.
This information is commonly represented in this form: `<nickname>!<user>@<host>:realname` 

If specified in [RPL_ISUPPORT (005)](RPL_ISUPPORT-(005)), the username and hostname length can be restricted.

- User modes ([more depth](https://www.unrealircd.org/docs/User_modes)).
	- It is not clear what if any user modes we need to implement.
	- The most common are:-
		- i
			- Invisible to all but those you share a channel with.
		- o
			- IRC operator (network, server operator, not channel op).
		- p
			- Private, hides channels you are a member of from WHOIS

Clients send messages to the server in the normal [message format](Message_format.md), with some caveats:
- As far as I can tell, clients do not send tags (@)
- "Clients MUST NOT include a source (:) when sending a message."
	- This makes sense as the source part tells the "true origin of the message"
	