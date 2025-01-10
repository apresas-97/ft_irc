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
	std::vector<t_message>	replies;
	t_message				reply;

	if (message.params.size() < 1)
	{
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname());
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	if (client->isAuthorised())
	{
		reply = createReply(ERR_ALREADYREGISTRED, ERR_ALREADYREGISTRED_STR, client->getNickname());
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	if (message.params[0] == this->_password)
	{
		std::cout << "Password correct... client is authorised" << std::endl;
		client->setAuthorised(true);
	}
	// Commented this out because it's not part of the RFC
	// else
	// {
	// 	reply = createReply(ERR_PASSWDMISMATCH, ERR_PASSWDMISMATCH_STR);
	// 	reply.target_client_fd = message.sender_client_fd;
	// 	reply.sender_client_fd = _serverFd;
	// 	replies.push_back(reply);
	// }
	return replies;
}

/*
Notes:

From the little information and references I found on this command's behavior,
it seems that:
	If the password provided is INCORRECT, the server should still accept
	the client's NICK and USER messages.
	Then, the servers should send the appropriate numeric reply errors that may
	have been triggered by the NICK and USER messages.
	After that, the server should send an ERROR message to the client to notify
	that the connection will be terminated.

	The first idea that comes to my mind would be to have the ERROR message
	be sent whenever the connection is about to be terminated.
	We might need a boolean flag in the client for that, and maybe check it
	at the beginning of the run_command function.
	But this is just an idea, we should discuss it.

	TODO: Discuss this behavior and implement it
*/