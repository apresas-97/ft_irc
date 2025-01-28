#include "Server.hpp"
#include <algorithm> // for std::find

/*
Command: NICK
Parameters: <nickname>
NICK command is used to give user a nickname or change the existing one.
*/
std::vector<t_message> Server::cmdNick( t_message & message ) 
{
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

	std::string nickname = message.params[0];

	// Check if the nickname follows the valid nickname format
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
		replies.push_back(createNotice(client, "Nickname " + nickname + " is already in use."));
		replies.push_back(createNotice(client, "You will be assigned a temporary unique nickname."));
		nickname = this->generateUniqueNickname();
		if (nickname == "***")
		{
			t_message quit_message;
			quit_message.command = "QUIT";
			quit_message.params.push_back("Nickname generation failed. Please try again later.");
			std::vector<t_message> quit_replies = this->cmdQuit(quit_message);
			replies.insert(replies.end(), quit_replies.begin(), quit_replies.end());
			return replies;
		}
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
