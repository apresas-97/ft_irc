#include "Server.hpp"

/*
Command: PONG
Parameters: <server> [ <server2> ]

PONG message is a reply to the PING message.

This is a quick implementation so the client doesn't disconnect.
TODO: Implement the PONG message properly.
*/
std::vector<t_message> Server::cmdPong( t_message & message ) 
{
	std::vector<t_message>	replies;
	t_message				reply;
	// Client * client = this->_current_client;

	// Param count check
	if (message.params.size() < 1) 
	{
		replies.push_back(createReply(ERR_NOORIGIN, ERR_NOORIGIN_STR));
		return replies;
	}

	// PONG reply
	reply.prefix = ":" + this->_name;
	reply.command = "PONG";
	reply.params.push_back(this->_name);
	if (message.params.size() > 1)
		reply.params.push_back(message.params[1]);
	reply.target_client_fds.insert(message.sender_client_fd);
	reply.sender_client_fd = _serverFd;
	replies.push_back(reply);

	return replies;
}
