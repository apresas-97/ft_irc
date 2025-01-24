#include "Server.hpp"

/*
Command: PONG
Parameters: <server1> [ <server2> ]

PONG message is a reply to ping message. If parameter <server2> is
given, this message MUST be forwarded to given target. The <server1>
parameter is the name of the entity who has responded to PING message
and generated this message.

Nowadays, the <server1> parameter is a token to echo back what was sent
in the PING message as <server1> parameter.

Since the PONG message is a reply to the PING message, users are allowed
to send PONG messages, and the possible errors will be checked and
replied accordingly, but the PONG message will not have any meaningful
effect. If formatted properly, no errors should be triggered, but no
answer will be sent back to the user.

Numeric Replies:
ERR_NOORIGIN : If no origin is specified (<server1> parameter)
ERR_NOSUCHSERVER: If the <server2> parameter does not match a server in the network.
*/
std::vector<t_message> Server::cmdPong( t_message & message ) 
{
	std::cout << "PONG MESSAGE RECEIVED" << std::endl;
	std::vector<t_message>	replies;
	Client * client = this->_current_client;

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	if (message.params.size() < 1) 
	{
		replies.push_back(createReply(ERR_NOORIGIN, ERR_NOORIGIN_STR));
		return replies;
	}

	if (message.params.size() > 1 && message.params[1] != this->getName())
	{
		replies.push_back(createReply(ERR_NOSUCHSERVER, ERR_NOSUCHSERVER_STR, message.params[1]));
		return replies;
	}

	if (message.params[0] == ":" + this->getName())
	{
		std::cout << "PARAMETER TOKEN IS VALID" << std::endl;
		std::cout << "set expected pong to false" << std::endl;
		client->setExpectedPong(false);
		client->setPongTimer();
		client->setLastActivity();
	}

	return replies;
}
