#include "Server.hpp"

static std::vector<std::string> parseMessage(const std::string &message, char delimiter) 
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(message);

	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

std::vector<t_message>	Server::cmdKick( t_message & message )
{
	std::vector<t_message> replies;

	Client * client = findClient(message.sender_client_fd);
	std::string nick = client->getNickname();
	std::string mainMsg = "";

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	if (message.params.size() < 2)
	{
		std::vector<std::string>	params;
		params.push_back(nick);
		params.push_back("KICK");
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, params));
		return replies;
	}

	std::vector<std::string> channel_targets = parseMessage(message.params[0], ',');
	for (std::vector<std::string>::iterator it = channel_targets.begin(); it != channel_targets.end(); ++it)
	{
		Channel	*	channel = findChannel(*it);

		if (!channel)
			replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, *it));
		else if (!channel->isUserInChannel(nick)) 
			replies.push_back(createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, *it));
		else if (!channel->isUserOperator(nick))
			replies.push_back(createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, *it));
	}

	if (message.params.size() >= 4)
	{
		for (size_t i = 3; i < message.params.size(); i++)
		{
			mainMsg += message.params[i];
			if (i < message.params.size() - 1)
				mainMsg += " ";
		}
	}
	else
	{
		mainMsg = nick + " has kicked user from channel";
	}

	std::vector<std::string> targets = parseMessage(message.params[1], ',');
	for (size_t i = 0; i < targets.size(); i++)
	{
		Client *	target_client = findClient(targets[i]);
		if (!target_client)
		{
			replies.push_back(createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, targets[i]));
			continue;
		}
		for (std::vector<std::string>::iterator it = channel_targets.begin(); it != channel_targets.end(); ++it)
		{
			Channel	*	channel = findChannel(*it);

			if (!channel)
				continue;
			if (!channel->isUserOperator(nick))
				continue;
			if (!channel->isUserInChannel(targets[i]))
			{
				std::vector<std::string>	params;
				params.push_back(targets[i]);
				params.push_back(*it);
				replies.push_back(createReply(ERR_USERNOTINCHANNEL, ERR_USERNOTINCHANNEL_STR, params));
				continue;
			}

			t_message kickMessage;
			kickMessage.prefix = ":" + client->getUserPrefix();
			kickMessage.command = "KICK";
			kickMessage.params.push_back(*it);
			kickMessage.params.push_back(targets[i]);
			kickMessage.params.push_back(mainMsg);
			kickMessage.sender_client_fd = client->getSocket();
			kickMessage.target_client_fds.insert(target_client->getSocket());
			addChannelToReply(kickMessage, channel);
			replies.push_back(kickMessage);

			channel->kickUser(targets[i]);
			channel->uninviteUser(targets[i]);

			target_client->removeChannel(*it);
			
			if (channel->isEmpty())
			{
				replies.push_back(createReply(RPL_CHANNELREMOVED, RPL_CHANNELREMOVED_STR, *it));
				removeChannel(*it);
			}
		}
	}

	return replies;
}
