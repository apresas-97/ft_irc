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
	// TODO
	// Until PRIVMSG is fully implemented I don't see a point in implementing NOTICE
}
