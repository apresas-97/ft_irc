#include "Server.hpp"

/*
Command: PASS
Parameters: <password>

The PASS command is used to set a 'connection password'.
The optional password can and MUST be set before any attempt to register
the connection is made. This requires that user send a PASS command before
sending the NICK/USER combination.
*/
std::vector<t_message> Server::cmdPass( t_message & message )
{
	std::cout << "PASS command called..." << std::endl;
	Client * client = this->_current_client;
	std::vector<t_message> replies;

	if (message.params.size() < 1)
	{
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname()));
		return replies;
	}
	if (message.params[0] == this->_password) 
	{
		std::cout << "Password correct... client is authorised" << std::endl;
		client->setAuthorised(true);
	}
	else
		replies.push_back(createReply(ERR_PASSWDMISMATCH, ERR_PASSWDMISMATCH_STR));
	return replies;
}

