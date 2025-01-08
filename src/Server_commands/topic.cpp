#include "Server.hpp"

std::vector<t_message> Server::cmdTopic(t_message &message) 
{
	std::cout << "TOPIC command called..." << std::endl;
	std::vector<t_message> replies;
	Client *client = this->_current_client;

	if (message.params.size() < 1) 
	{
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR));
		return replies;
	}

	std::string channelName = message.params[0];

	if (this->_channels.find(channelName) == this->_channels.end()) 
	{
//		replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, {client->getNickname(), channelName})); // incorrect call
		return replies;
	}

	Channel &channel = this->_channels[channelName];

	if (!channel.isUserInChannel(client->getNickname())) 
	{
//		replies.push_back(createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, {client->getNickname(), channelName})); // incorrect call
		return replies;
	}

	if (message.params.size() == 1) 
	{
		if (channel.getTopic().empty()) 
		{
//			replies.push_back(createReply(RPL_NOTOPIC, RPL_NOTOPIC_STR, {client->getNickname(), channelName})); // incorrect call
		} 
		else 
		{
//			replies.push_back(createReply(RPL_TOPIC, RPL_TOPIC_STR, {client->getNickname(), channelName, channel.getTopic()})); // incorrect call
		}
	} 
	else 
	{
		std::string newTopic = message.params[1];
		for (size_t i = 2; i < message.params.size(); i++) 
		{
			newTopic += " " + message.params[i];
		}

		if (channel.getMode('t') && !channel.isUserOperator(client->getNickname())) 
		{
//			replies.push_back(createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, {client->getNickname(), channelName})); // incorrect call
			return replies;
		}

		if (newTopic == ":") 
		{
			channel.setTopic("");
		} else {
			channel.setTopic(newTopic.substr(0, newTopic.size()));
		}

		t_message topicMessage;
		topicMessage.prefix = client->getUserPrefix();
		topicMessage.command = "TOPIC";
		topicMessage.params.push_back(channelName);
		topicMessage.params.push_back(channel.getTopic());
		// topicMessage.sender_client_fd = client->getFd();
		topicMessage.target_channels.push_back(&channel);
		// channel.broadcastMessage(topicMessage, client->getNickname());

//		replies.push_back(createReply(RPL_TOPIC, RPL_TOPIC_STR, {client->getNickname(), channelName, channel.getTopic()})); // incorrect call
	}

	return replies;
}
