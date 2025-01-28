#include "Server.hpp"

template <typename T>
std::string to_string(T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

static std::vector<std::string> parseMessage(const std::string &message, char delimiter) 
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(message);

	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

/*
	Command: LIST
	Parameters: [<channel>{,<channel>} [<target>]]

	The LIST command is used to list channels and their topics. 
	Without parameters, it displays all currently visible channels. 
	If a specific channel or multiple channels are provided as parameters, it only lists those.
	This command is particularly useful for discovering available channels on the server.

	- If the client is not registered, an error message is returned.
	- If the channel does not exist or is hidden, it will not be displayed.
	- Channels with mode +s (secret) are only visible to members.

	This implementation is part of the ft_irc project, a 42Barcelona school project,
	based on the original work of irssi contributors.
*/

std::vector<t_message> Server::cmdList(t_message &message) 
{
	std::vector<t_message> replies;
	t_message reply;
	Client *client = this->_current_client;

	if (client->isRegistered() == false) 
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	if (message.params.empty()) 
	{
		for (std::map<std::string, Channel>::iterator it = this->_channels.begin(); it != this->_channels.end(); ++it) 
		{
			Channel &channel = it->second;

			if (channel.getMode('s') && !channel.isUserInChannel(client->getNickname())) 
				continue;

			std::vector<std::string> params;
			params.push_back(channel.getName());
			params.push_back(to_string(channel.getUserCount()));
			params.push_back(channel.getTopic());
			reply = createReply(RPL_LIST, RPL_LIST_STR, params);
			replies.push_back(reply);
		}
	} 
	else 
	{
		std::vector<std::string> channels = parseMessage(message.params[0], ',');
		for (size_t i = 0; i < channels.size(); ++i) 
		{
			std::string channelName = channels[i];

			if (!channelFound(channelName)) 
			{
				reply = createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, channelName);
				replies.push_back(reply);
				continue;
			}

			Channel *channel = findChannel(channelName);

			if (channel->getMode('s') && !channel->isUserInChannel(client->getNickname())) 
				continue;

			std::vector<std::string> params;
			params.push_back(channel->getName());
			params.push_back(to_string(channel->getUserCount()));
			params.push_back(channel->getTopic());
			reply = createReply(RPL_LIST, RPL_LIST_STR, params);
			replies.push_back(reply);
		}
	}

	reply = createReply(RPL_LISTEND, RPL_LISTEND_STR);
	replies.push_back(reply);

	return replies;
}
