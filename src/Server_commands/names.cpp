#include "Server.hpp"
/*
	Command: NAMES
	Parameters: [ <channel> *( "," <channel> ) ]

	The NAMES command is used by a user to request the list of users currently in specific channels.

	If no channel is specified, the server replies with a list of users from all channels that the user is allowed to see.

	If a channel is specified, the server will respond with the list of users in that channel, provided the user is a member of it.

	A special response with RPL_NAMREPLY will be sent for each channel, listing the users currently in the channel.

	If the user is not a member of the specified channel, the server will respond with ERR_NOTONCHANNEL.

	If the channel does not exist, the server will respond with ERR_NOSUCHCHANNEL.

	The response will include the end of the list with RPL_ENDOFNAMES once the complete list of users in the channel(s) is sent.

	If no parameters are given (i.e., no channels are specified), the server replies with the users of all channels the client is allowed to see.

	A client that is not registered will receive an ERR_NOTREGISTERED error message.
*/

std::vector<t_message> Server::cmdNames(t_message &message)
{
	std::vector<t_message> replies;

	Client *client = this->_current_client;

	if (!client->isRegistered())
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	// If no channel is specified, show all visible users in all channels
	if (message.params.empty())
	{
		// Iterate through all channels
		for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			std::vector<std::string> clientList = it->second.getUsersOpClean();
			if (!clientList.empty())
			{
				std::vector<std::string> paramsName;
				paramsName.push_back(it->first);
				std::string names;
				for (size_t i = 0; i < clientList.size(); ++i)
				{
					names += clientList[i] + " ";
				}
				paramsName.push_back(names);
				t_message nameReply = createReply(RPL_NAMREPLY, RPL_NAMREPLY_STR, paramsName);
				addChannelToReply(nameReply, &it->second);
				replies.push_back(nameReply);

				// Final response for the list of names in the channel
				std::vector<std::string> paramsEnd = clientList;
				paramsEnd.push_back(it->first);
				t_message endOfNames = createReply(RPL_ENDOFNAMES, RPL_ENDOFNAMES_STR, paramsEnd);
				addChannelToReply(endOfNames, &it->second);
				replies.push_back(endOfNames);
			}
		}
	}
	else
	{
		// If a specific channel is specified
		std::string channelName = message.params[0];
		Channel *channel = findChannel(channelName);

		if (channel == NULL)
		{
			replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, channelName));
			return replies;
		}

		if (!channel->isUserInChannel(client->getNickname()))
		{
			replies.push_back(createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, channelName));
			return replies;
		}

		std::vector<std::string> clientList = channel->getUsersOpClean();
		if (!clientList.empty())
		{
			// Generate reply with the list of users in the specified channel
			std::vector<std::string> paramsName = clientList;
			paramsName.push_back(channelName);
			t_message nameReply = createReply(RPL_NAMREPLY, RPL_NAMREPLY_STR, paramsName);
			addChannelToReply(nameReply, channel);
			replies.push_back(nameReply);

			// Final response indicating the end of the list of names in the channel
			std::vector<std::string> paramsEnd = clientList;
			paramsEnd.push_back(channelName);
			t_message endOfNames = createReply(RPL_ENDOFNAMES, RPL_ENDOFNAMES_STR, paramsEnd);
			addChannelToReply(endOfNames, channel);
			replies.push_back(endOfNames);
		}
	}

	return replies;
}
