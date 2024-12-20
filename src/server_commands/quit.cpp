#include "Server.hpp"

/*
	Command: QUIT
	Parameters: [<Quit message>]
	A client session is terminated with a QUIT message.
	The server acknowledges this by sending an ERROR message to the client->
	This command has no numeric replies.
*/
std::vector<t_message> Server::cmdQuit( t_message & message )
{
	std::cout << "QUIT command called..." << std::endl;
	/*
	I tested this command on a few servers and the one that seems to more
	closely follow the RFC docs is DALnet:
	// With quit message
	<< QUIT :I gotta go grocery shopping, I'll be back later
	>> ERROR :Closing Link: my_hostname (Quit: I gotta go grocery shopping, I'll be back later)
	// Without quit message
	<< QUIT
	>> ERROR :Closing Link: my_hostname (Quit: mynickname)

	I connected to the same server and the same channel with another client and
	quit the same way, and the message is broadcasted to all users in the channel
	This is what I received on the other client:

	>> :otherprefix QUIT :Quit: I gotta go grocery shopping, I'll be back later

	*/
	std::vector<t_message> replies;
	t_message error_acknowledgement; // Message for the client that is quitting
	t_message quit_broadcast; // Message for the clients in the same channels as the quitting client
	Client & client = *this->_current_client;

	// TODO: Have a method to get the fd's of all the clients in the same channels as a particular
	// client, we will store that in a std::vector<int> and store that in the t_message struct

	// TODO: Update the t_message struct so the target fd's are stored in a std::vector<int>
	// When sending a t_message, the server will send the message to all the fd's in the vector
	// When only one target is needed, the vector will have only one element, so it will be fine

	std::string quit_message;
	if (message.params.size() == 0)
		quit_message = client->getNickname();
	else
		quit_message = message.params[0];

	error_acknowledgement.prefix = this->getName();
	error_acknowledgement.command = "ERROR";
	error_acknowledgement.params.push_back(":Closing Link: " + client->getHostname() + " (Quit: " + quit_message + ")");
	// error_acknowledgement.target_client_fd = ... // This is the TODO part (but this will only be for the sender)

	quit_broadcast.prefix = client->getPrefix();
	quit_broadcast.command = "QUIT";
	quit_message = ":Quit: " + quit_message;
	quit_broadcast.params.push_back(quit_message);
	// quit_broadcast.target_client_fd = ... // This is the TODO part

	replies.push_back(error_acknowledgement);
	replies.push_back(quit_broadcast);
	// Test...
	for (size_t i = 0; i < _poll_fds.size(); i++)
	{
		if (_poll_fds[i].fd == message.sender_client_fd)
		{
			close(_poll_fds[i].fd);
			// Remove from _poll_fds .	std::vector<struct pollfd>()
			// Remove from _clients ... std::map<int, Client>()
			break ;
		}
	}
	return replies;
}