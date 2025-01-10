#include "Server.hpp"

/*
Command: NOTICE
Parameters: <msgtarget> <text>

The NOTICE message works almost exactly like PRIVMSG.
The only difference is that no automatic reply should be sent
in response to a NOTICE message.

It's purpose is avoiding loops between clients automatically sending
eachother automatic replies.
*/
std::vector<t_message>	Server::cmdNotice( t_message & message )
{
	// Possible implementation? I don't know if it will be so simple
	std::vector<t_message> replies = cmdPrivMsg(message);
	for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
	{
		if (it->command == "PRIVMSG")
			it->command = "NOTICE";
	}
	return replies;
}
