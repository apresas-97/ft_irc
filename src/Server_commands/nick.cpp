#include "Server.hpp"
#include <algorithm> // for std::find
#include <cstdlib>

static std::string generateRandomNickname( void )
{
	const std::string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
	const size_t charsSize = chars.size();
	std::string randomString;

	for (size_t i = 0; i < 8; ++i) {
		randomString += chars[rand() % charsSize];
	}

	return randomString;
}

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

	// Check if the client is restricted
	if (client->getMode('r') == true)
	{
		reply = createReply(ERR_RESTRICTED, ERR_RESTRICTED_STR);
		replies.push_back(reply);
		return replies;
	}

	// Check if the client has provided a nickname
	if (message.params.size() < 1) 
	{
		reply = createReply(ERR_NONICKNAMEGIVEN, ERR_NONICKNAMEGIVEN_STR);
		replies.push_back(reply);
		return replies;
	}

	// Check if the nickname follows the valid nickname format
	std::string nickname = message.params[0];
	if (isValidNickname(nickname) == false)
	{
		reply = createReply(ERR_ERRONEUSNICKNAME, ERR_ERRONEUSNICKNAME_STR, nickname);
		replies.push_back(reply);
		return replies;
	}

	// If the given nickname is the same as the current nickname, do nothing
	if (client->getNickname() == nickname) 
		return replies;

	// Nickname is already taken ?
	if (this->isNicknameTaken(nickname) == true)
	{
		reply = createReply(ERR_NICKNAMEINUSE, ERR_NICKNAMEINUSE_STR, nickname);
		replies.push_back(reply);
		replies.push_back(createNotice(client, "You have been assigned a provisional random nickname"));
		while (true)
		{
			nickname = generateRandomNickname();
			// TODO
			// generate the random name in a loop until the resulting nickname is not taken then break and NOT RETURN REPLIES HERE
		}
		return replies;
	}

	std::string old_prefix = client->getUserPrefix();

	this->updateClientNickname(client, nickname);

	// If the client is not yet registered, no acknowledgement is needed
	if (client->isRegistered() == false)
		return replies;

	t_message acknowledgement;
	acknowledgement.prefix = ":" + old_prefix;
	acknowledgement.command = "NICK";
	acknowledgement.params.push_back(nickname);
	acknowledgement.target_client_fds.insert(client->getSocket());
	replies.push_back(acknowledgement);

	// Send a broadcast message to all users in the same channels as the user
	t_message nick_broadcast;
	nick_broadcast.prefix = ":" + old_prefix;
	nick_broadcast.command = "NICK";
	nick_broadcast.params.push_back(nickname);
	std::vector<Channel *> channels = client->getChannelsVector();
	for (std::vector<Channel *>::iterator it = channels.begin(); it != channels.end(); ++it) 
		addChannelToReplyExcept(nick_broadcast, *it);
	replies.push_back(nick_broadcast);

	return replies;
}
