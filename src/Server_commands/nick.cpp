#include "Server.hpp"
#include <algorithm> // for std::find

/*
Command: NICK
Parameters: <nickname>
NICK command is used to give user a nickname or change the existing one.
*/
std::vector<t_message> Server::cmdNick( t_message & message ) 
{
	std::cout << "NICK command called..." << std::endl;
	Client * client = this->_current_client;
	std::vector<t_message>	replies;
	t_message				reply;

	// Check if the client is authorised and registered
	if (client->isAuthorised() == false || client->isRegistered() == false) 
	{
		reply = createReply(ERR_RESTRICTED, ERR_RESTRICTED_STR);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}

	// Check if the client is in restricted mode ( we'll see what we do with this )
	if (client->getMode('r') == true) 
	{
		reply = createReply(ERR_RESTRICTED, ERR_RESTRICTED_STR);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}

	// Check if the client has provided a nickname
	if (message.params.size() < 1) 
	{
		reply = createReply(ERR_NONICKNAMEGIVEN, ERR_NONICKNAMEGIVEN_STR);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}

	// Check if the nickname follows the valid nickname format
	std::string nickname = message.params[0];
	if (irc_isValidNickname(nickname) == false) 
	{
		reply = createReply(ERR_ERRONEUSNICKNAME, ERR_ERRONEUSNICKNAME_STR, nickname);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}

	// If the given nickname is the same as the current nickname, do nothing
	if (client->getNickname() == nickname) 
		return replies;

	// Nickname is already taken ?
	std::vector<std::string>::iterator it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), nickname);
	if (it != this->_taken_nicknames.end()) 
	{
		reply = createReply(ERR_NICKNAMEINUSE, ERR_NICKNAMEINUSE_STR, nickname);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}

	// Store the user's prefix before changing the nickname, for later use
	std::string old_prefix = client->getPrefix();

	// Remove the old nickname from the taken nicknames list
	std::string old_nickname = client->getNickname();
	if (old_nickname.empty() == false) 
	{
		it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), old_nickname);
		if (it != this->_taken_nicknames.end()) 
			this->_taken_nicknames.erase(it);
	}

	// Set the new nickname
	client->setNickname(nickname);
	this->_taken_nicknames.push_back(nickname);

	// Send a broadcast message to all users in the same channels as the user
	// The user itself is included at the message works as an acknowledgement
	t_message nick_broadcast;
	nick_broadcast.prefix = old_prefix;
	nick_broadcast.command = "NICK";
	nick_broadcast.params.push_back(":" + nickname);

	std::vector<Channel *> channels = client->getChannelsVector();

	for (std::vector<Channel *>::iterator it = channels.begin(); it != channels.end(); ++it) 
	{
		std::vector<int> channel_users_fds = (*it)->getFds("users");
		for (std::vector<int>::iterator it2 = channel_users_fds.begin(); it2 != channel_users_fds.end(); ++it2) 
			nick_broadcast.target_client_fds.insert(*it2);
	}
	replies.push_back(nick_broadcast);

	delete client; // TODO: REMEMBER TO FIX THIS, CLIENT SHOULD NOT BE DELETED HERE
	return replies; // TEMP SOLUTION IN ORDER TO COMPILE PROJECT
}
