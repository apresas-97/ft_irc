#include "Server.hpp"

/*
Command: CAP
This command is used to negotiate capabilities between the client and the server.
We will only accept a CAP message at the beginning of the connection and we will reply
with a CAP message with no capabilities.
*/
std::vector<t_message>	Server::cmdCap( t_message & message )
{
	std::vector<t_message>	replies;
	Client * client = this->_current_client;

	// We expect a CAP LS command
	if (message.params.size() < 1 || message.params[0] != "LS")
	{
		// I don't know what to do if it's not LS
		return replies;
	}

	t_message reply;
	reply.prefix = ":" + this->getName();
	reply.command = "CAP";
	std::string nickname = client->getNickname();
	if (nickname.empty())
		nickname = "*";
	reply.params.push_back(nickname);
	reply.params.push_back("LS");
	reply.params.push_back(""); // EVAL
	reply.target_client_fds.insert(client->getSocket());
	replies.push_back(reply);

	return replies;
}

