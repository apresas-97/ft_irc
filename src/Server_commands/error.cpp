#include "Server.hpp"

/*
Command: ERROR
Parameters: <error message>

This command is used mainly for server-to-server communication, which we don't
care about.

The ERROR message is also used before terminating a client connection.

This command is only for internal use, if the user send an ERROR message,
it will be silently ignored. That should be handled in the run_command function.

This command has a quirk, its message does not have a prefix, only the ERROR command and
the error message.
TODO Check if not having a prefix works when sending the message to the client.
*/
std::vector<t_message>	Server::cmdError( std::string & error_message )
{
	Client * client = this->_current_client;
	std::vector<t_message> replies;
	t_message error;

	error.command = "ERROR";
	error.params.push_back(error_message);
	error.sender_client_fd = this->_serverFd; // TODO: We should check if this variable is necessary at all anymore
	error.target_client_fds.insert(client->getSocket());
	replies.push_back(error);

	return replies;
}