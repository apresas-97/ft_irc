#include "Server.hpp"
/*

	0. Management
	1. Validation
		Does it have a recipient? Args < 1
		Does it have enough text? Args < 2
	3. Extract targets
	4. Prepare the text to send
		Iterate over the text and add a space to each parameter of tmessage->params
	5. Iterate over the targets
		5.a. If # or & (Channels)
			5.a.a. The channel exists
			5.a.b. The channel does NOT exist
		5.b. Bot
		5.c. The target nickname exists
			5.c.a. The nickname does not exist

*/

// Helper function that splits a string into tokens based on a delimiter
// Returns a vector containing all the tokens
static std::vector<std::string> parseMessage(const std::string &message, char delimiter)
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(message);
	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);
	return tokens;
}

// Handles the PRIVMSG command in IRC which is used to send messages to users or channels
// Returns a vector of replies to be sent back to clients
std::vector<t_message> Server::cmdPrivMsg(t_message &message)
{
	std::cout << "PRIVMSG command called..." << std::endl;
	std::vector<t_message> replies;
	t_message reply;
	Client *client = _current_client;
	std::vector<std::string> targets;
	std::string textToSend;

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	// Check if there are any parameters (recipient) in the message
	if (message.params.size() < 1)
	{
		// If no recipient specified, send error reply
		std::vector<std::string> params;
		params.push_back(client->getNickname());
		params.push_back("PRIVMSG");
		reply = createReply(ERR_NORECIPIENT, ERR_NORECIPIENT_STR, params);
		replies.push_back(reply);
		return replies;
	}

	// Split the targets string by commas to handle multiple recipients
	targets = parseMessage(message.params[0], ',');

	// Check if there's actual message content
	if (message.params.size() < 2)
	{
		// If no message content, send error reply
		reply = createReply(ERR_NOTEXTTOSEND, ERR_NOTEXTTOSEND_STR, client->getNickname());
		replies.push_back(reply);
		return replies;
	}

	textToSend = message.params[1];

	// Process each target (recipient) in the message
	for (size_t i = 0; i < targets.size(); i++)
	{
		std::string & target = targets[i];
		
		// Handle channel messages (targets starting with # or &)
		if (isValidChannelName(target))
		{
			// Check if channel exists
			if (_channels.find(target) != _channels.end())
			{
				Channel * channel = this->findChannel(target);
				// Check if sender is in the channel
				if (!channel->isUserInChannel(client->getNickname()))
				{
					// If sender not in channel, send error reply
					std::cout << "User is not in channel..." << std::endl;
					std::vector<std::string> params;
					params.push_back(client->getNickname());
					params.push_back(target);
					reply = createReply(ERR_CANNOTSENDTOCHAN, ERR_CANNOTSENDTOCHAN_STR, params);
					replies.push_back(reply);
					continue;
				}
				// Create and send message to channel
				t_message channelMessage;
				channelMessage.prefix = ":" + client->getUserPrefix();
				channelMessage.command = "PRIVMSG";
				channelMessage.params.push_back(target);
				channelMessage.params.push_back(textToSend);
				channelMessage.sender_client_fd = client->getSocket();
				addChannelToReplyExcept(channelMessage, channel);
				replies.push_back(channelMessage);
			}
			else
			{
				// If channel doesn't exist, send error reply
				std::vector<std::string> params;
				params.push_back(client->getNickname());
				params.push_back(target);
				reply = createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, params);
				replies.push_back(reply);
			}
		}
		// Handle private messages to users
		else if (Client *targetClient = findClient(target))
		{
			// Create and send private message
			t_message privateMessage;
			privateMessage.prefix = ":" + client->getUserPrefix();
			privateMessage.command = "PRIVMSG";
			privateMessage.params.push_back(target);
			privateMessage.params.push_back(textToSend);
			privateMessage.sender_client_fd = this->_current_client->getSocket();
			privateMessage.target_client_fds.insert(targetClient->getSocket());
			replies.push_back(privateMessage);
		}
		else
		{
			// If target user doesn't exist, send error reply
			std::vector<std::string> params;
			params.push_back(client->getNickname());
			params.push_back(target);
			reply = createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, params);
			replies.push_back(reply);
		}
	}
	return replies;
}
