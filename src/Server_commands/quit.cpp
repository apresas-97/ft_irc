#include "Server.hpp"

/*
	Command: QUIT
	Parameters: [<Quit message>]
	A client session is terminated with a QUIT message.
	The server acknowledges this by sending an ERROR message to the client.
	This command has no numeric replies.
*/
std::vector<t_message> Server::cmdQuit( t_message & message )
{
	std::vector<t_message>	replies;
	t_message error_acknowledgement;
	t_message quit_broadcast;
	Client * client = this->_current_client;

	std::string quit_message;
	if (message.params.size() == 0)
		quit_message = "leaving";
	else
		quit_message = message.params[0];

	error_acknowledgement.command = "ERROR";
	error_acknowledgement.params.push_back("Closing Link: " + client->getHostname() + " (" + quit_message + ")");
	error_acknowledgement.target_client_fds.insert(client->getSocket());
	replies.push_back(error_acknowledgement);

	quit_broadcast.prefix = ":" + client->getUserPrefix();
	quit_broadcast.command = "QUIT";
	quit_broadcast.params.push_back(quit_message);
	std::vector<Channel *> channels = client->getChannelsVector();
	for (std::vector<Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		addChannelToReplyExcept(quit_broadcast, *it);
		(*it)->kickUser(client->getNickname());

		if ((*it)->isEmpty())
			this->removeChannel((*it)->getName());
	}
	this->uninviteUser(client->getNickname());
	replies.push_back(quit_broadcast);

	client->setTerminate(true);
	return replies;
}

