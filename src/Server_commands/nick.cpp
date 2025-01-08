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

	if (client->isAuthorised() == false || client->isRegistered() == false) 
	{
		reply = createReply(ERR_RESTRICTED, ERR_RESTRICTED_STR);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	if (client->getMode('r') == true) 
	{
		reply = createReply(ERR_RESTRICTED, ERR_RESTRICTED_STR);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	if (message.params.size() < 1) 
	{
		reply = createReply(ERR_NONICKNAMEGIVEN, ERR_NONICKNAMEGIVEN_STR);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	std::string nickname = message.params[0];
	if (irc_isValidNickname(nickname) == false) 
	{
		reply = createReply(ERR_ERRONEUSNICKNAME, ERR_ERRONEUSNICKNAME_STR, nickname);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}

	// apresas-: Check for nickname collision ? Idk how to do this yet
	// This triggers ERR_NICKCOLLISION

	// apresas-: Check if the nickname cannot be chosen because of the nick delay mechanism
	// I don't even know if we'll implement that yet
	// If the name is not available, return ERR_UNAVAILRESOURCE

	std::vector<std::string>::iterator it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), nickname);
	if (it != this->_taken_nicknames.end()) 
	{
		reply = createReply(ERR_NICKNAMEINUSE, ERR_NICKNAMEINUSE_STR, nickname);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	// If all went well, change the user's nickname and return its own message as acknowledgement
	client->setNickname(nickname);
	this->_taken_nicknames.push_back(nickname);
	// TODO create reply when everything went OK

	// It should now be removed from the taken nicknames list, but not immediately
	// I need to look this up and see how it truly works
	// I think it's removed after a certain amount of time, but I'm not sure

	/*
	apresas-: TODO: Send a message to all channels the user is in that the nickname has changed
	The message should be something like:
	:oldnickname NICK newnickname
	It should be broadcasted to all users in the same channels as the user
	And probably only to those that would be allowed to see the user???? Idk
	I need to look this up
	Ughhhhh
	*/
	delete client;
	return replies; // TEMP SOLUTION IN ORDER TO COMPILE PROJECT
}
