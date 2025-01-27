#include "Server.hpp"
/*

	PSEUDO

	1. Access the appropriate client (Executor)
		- Get their nickname
	2. Prepare the kick message

	3. Error handling
		- Not enough parameters
		- Channel does not exist
		- Client is not in the channel
		- Client is not an operator: cannot kick anyone
	4. If there are 4 or more parameters
		- Assemble the kick message

	5. Handle targets
		- Errors
			- Target nickname does not exist
			- Target to be kicked is not in the channel
		
		- Send message
		- Remove the client from all lists

	6. Delete the channel if it has no more users

	Format

	KICK <channel> <user> [<reason>]
	KICK <channel> <user1,user2,user3> [<reason>] 

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


std::vector<t_message>	Server::cmdKick( t_message & message )
{
	std::cout << "KICK command called..." << std::endl;
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
		mainMsg = nick + " has kicked user from channel"; // ??
	}

	std::vector<std::string> targets = parseMessage(message.params[1], ',');
	for (size_t i = 0; i < targets.size(); i++)
	{
		if (!findClient(targets[i]))
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
			kickMessage.target_client_fds.insert(findClient(targets[i])->getSocket());
			addChannelToReply(kickMessage, channel);
			replies.push_back(kickMessage);

			channel->kickUser(targets[i]);
			channel->uninviteUser(targets[i]);
			// TODO client remove channels from channels vector
			if (channel->isEmpty())
			{
				replies.push_back(createReply(RPL_CHANNELREMOVED, RPL_CHANNELREMOVED_STR, *it));
				removeChannel(*it);
			}
		}
	}

	return replies;
}
