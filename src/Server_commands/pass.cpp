#include "Server.hpp"

/*
Command: PASS
Parameters: <password>

The PASS command is used to set a 'connection password'.
The optional password can and MUST be set before any attempt to register
the connection is made. This requires that user send a PASS command before
sending the NICK/USER combination.

Numeric Replies:
- ERR_NEEDMOREPARAMS: If no parameters are given
- ERR_ALREADYREGISTRED: If the user has already registered
*/
std::vector<t_message> Server::cmdPass( t_message & message )
{
	std::cout << "PASS command called..." << std::endl;
	Client * client = this->_current_client;
	std::vector<t_message>	replies;
	t_message				reply;

	if (message.params.size() < 1)
	{
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname());
		replies.push_back(reply);
		return replies;
	}
	if (client->isAuthorised())
	{
		reply = createReply(ERR_ALREADYREGISTRED, ERR_ALREADYREGISTRED_STR, client->getNickname());
		replies.push_back(reply);
		return replies;
	}
	client->setPassword(message.params[0]);
	return replies;
}
