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
	std::cout << "QUIT command called..." << std::endl;

	std::vector<t_message>	replies;
	t_message error_acknowledgement; // Message for the client that is quitting
	t_message quit_broadcast; // Message for the clients in the same channels as the quitting client
	Client * client = this->_current_client;

	std::string quit_message;
	if (message.params.size() == 0)
		quit_message = client->getNickname();
	else
		quit_message = message.params[0];

	// error_acknowledgement.prefix = ":" + this->getName();
	error_acknowledgement.command = "ERROR";
	error_acknowledgement.params.push_back("Closing Link: " + client->getHostname() + " (" + quit_message + ")");
	error_acknowledgement.target_client_fds.insert(client->getSocket());
	replies.push_back(error_acknowledgement);

	// quit_broadcast.prefix = ":" + client->getUserPrefix();
	quit_broadcast.command = "QUIT";
	quit_broadcast.params.push_back(quit_message);
	std::vector<Channel *> channels = client->getChannelsVector();
	std::cout << "vector of channels size: " << channels.size() << std::endl;
	for (std::vector<Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
	{
		// Channel * channel = *it;
		
		// if (!channel)
		// 	std::cout << "Uh oh, channel is NULL" << std::endl;
		std::cout << "Removing user from channel: " << (*it)->getName() << std::endl;
		std::cout << "Users in channel:" << std::endl;
		std::vector<std::string> chan_users = (*it)->getUsers();
		for (std::vector<std::string>::iterator it = chan_users.begin(); it != chan_users.end(); ++it)
		{
			std::cout << *it << std::endl;
		}
		std::cout << "User list end" << std::endl;
		
		addChannelToReplyExcept(quit_broadcast, *it);
		(*it)->kickUser(client->getNickname());

		if ((*it)->isEmpty()) {
			std::cout << "Channel " << (*it)->getName() << " is empty. Deleting channel." << std::endl;
			this->removeChannel((*it)->getName());
		}
	}
	replies.push_back(quit_broadcast);

	client->setTerminate(true);
	return replies;
}

