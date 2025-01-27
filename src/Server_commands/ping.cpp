#include "Server.hpp"

/*
Command: PING
Parameters: <server1> [ <server2> ]

The PING command is used to test the presence of a connection.
When a PING message is received, the appropriate PONG message MUST be sent as reply to <server1>
(server which sent the PING message out) as soon as possible.

IF the <server2> parameter is specified, it represents the TARGET of the ping, and the message
gets forwarded there.
This second part is server-to-server communication, and it's not implemented in this project.
But it is correct to specify the second parameter in the PING message.
In our case, it should only be the server's name, otherwise it will trigger the appropriate
numeric reply error.

NOTE: Nowadays, the server1 parameter is mostly used as a token to echo back in the PONG message.

Numeric Replies:
ERR_NOORIGIN : If no origin is specified (<server1> parameter)
ERR_NOSUCHSERVER: If the <server2> parameter does not match a server in the network.
(In our case, the network only contains this one server, <server2> must be the server's name)
*/
std::vector<t_message>	Server::cmdPing( t_message & message )
{
	std::cout << "PING MESSAGE RECEIVED" << std::endl;
	std::vector<t_message> replies;
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

	std::string token = message.params[0];
	if (message.params[0] == this->getName())
		token = client->getNickname();

	t_message pong;
	pong.prefix = ":" + this->getName();
	pong.command = "PONG";
	pong.params.push_back(this->getName());
	pong.params.push_back(token);
	pong.target_client_fds.insert(client->getSocket());
	pong.sender_client_fd = this->_serverFd;
	replies.push_back(pong);

	return replies;
}
