#include "Server.hpp"

/*
	Command: PART
	Parameters: <channel> *( "," <channel> ) [ <message> ]

	The PART command allows a user to leave one or more channels. 
	If the user successfully leaves a channel, a PART message is sent to the channel and to the client.
*/

static std::vector<std::string> parseMessage(const std::string &message, char delimiter) 
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(message);

	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

std::vector<t_message> Server::cmdPart(t_message &message)
{
	std::cout << "PART command called..." << std::endl;
	std::vector<t_message> replies;
	t_message reply;

	Client *client = this->_current_client;

	// Check if the client is registered
	if (!client->isRegistered())
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	// Check if required parameters are provided
	if (message.params.empty())
	{
		std::vector<std::string> params;
		params.push_back(client->getNickname());
		params.push_back("PART");
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, params);
		replies.push_back(reply);
		return replies;
	}

	// Parse channel names
	std::vector<std::string> channel_names = parseMessage(message.params[0], ',');
	std::string part_message = (message.params.size() > 1) ? message.params[1] : "";

	for (size_t i = 0; i < channel_names.size(); i++)
	{
		std::string currentChannel = channel_names[i];

		// Check if the channel exists
		Channel *channel = findChannel(currentChannel);
		if (!channel)
		{
			reply = createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, currentChannel);
			replies.push_back(reply);
			continue;
		}

		// Check if the user is in the channel
		if (!channel->isUserInChannel(client->getNickname()))
		{
			reply = createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, currentChannel);
			replies.push_back(reply);
			continue;
		}

		// Prepare PART response message
		t_message partMessage;
		partMessage.prefix = ":" + client->getUserPrefix();
		partMessage.command = "PART";
		partMessage.params.push_back(currentChannel);
		partMessage.sender_client_fd = client->getSocket();
		partMessage.target_client_fds.insert(message.sender_client_fd);
		addChannelToReply(partMessage, channel);
		replies.push_back(partMessage);

		// Remove the user from the channel
		channel->kickUser(client->getNickname());
		this->_current_client->removeChannel(currentChannel);
		// Delete the channel if it is empty
		if (channel->getUserCount() == 0)
		{
			std::cout << "Channel " << currentChannel << " is now empty. Deleting..." << std::endl;
			removeChannel(currentChannel);
			continue;
		}
	}

	return replies;
}
