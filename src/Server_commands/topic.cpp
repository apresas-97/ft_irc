#include "Server.hpp"

std::vector<t_message> Server::cmdTopic(t_message &message) 
{
	std::cout << "TOPIC command called..." << std::endl;
	std::vector<t_message>	replies;
	t_message				reply;
	Client *client = this->_current_client;

	if (message.params.size() < 1) 
	{
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR);
		replies.push_back(reply);
		return replies;
	}

	std::string channelName = message.params[0];

	if (this->_channels.find(channelName) == this->_channels.end()) 
	{
		std::vector<std::string> params;
		params.push_back(client->getNickname());
		params.push_back(channelName);
		reply = createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, params);
		replies.push_back(reply);
		return replies;
	}

	Channel * channel = this->_channels[channelName];

	if (!channel->isUserInChannel(client->getNickname())) 
	{
		std::vector<std::string> params;
		params.push_back(client->getNickname());
		params.push_back(channelName);
		reply = createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, params);
		replies.push_back(reply);
		return replies;
	}

	if (message.params.size() == 1) 
	{
		if (channel->getTopic().empty()) 
		{
			std::vector<std::string> params;
			params.push_back(client->getNickname());
			params.push_back(channelName);
			reply = createReply(RPL_NOTOPIC, RPL_NOTOPIC_STR, params);
			replies.push_back(reply);
		} 
		else 
		{
			std::vector<std::string> params;
			params.push_back(client->getNickname());
			params.push_back(channelName);
			params.push_back(channel->getTopic());
			reply = createReply(RPL_TOPIC, RPL_TOPIC_STR, params);
			replies.push_back(reply);
		}
	} 
	else 
	{
		std::string newTopic = message.params[1];
		for (size_t i = 2; i < message.params.size(); i++) 
		{
			newTopic += " " + message.params[i];
		}

		if (channel->getMode('t') && !channel->isUserOperator(client->getNickname())) 
		{
			std::vector<std::string> params;
			params.push_back(client->getNickname());
			params.push_back(channelName);
			reply = createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, params);
			replies.push_back(reply);
			return replies;
		}

		if (newTopic == ":") 
		{
			channel->setTopic("");
		} 
		else 
		{
			channel->setTopic(newTopic.substr(0, newTopic.size()));
		}

		t_message topicMessage;
		topicMessage.prefix = client->getUserPrefix();
		topicMessage.command = "TOPIC";
		topicMessage.params.push_back(channelName);
		topicMessage.params.push_back(channel->getTopic());
		topicMessage.sender_client_fd = client->getSocket();
		addChannelToReply(topicMessage, channel);

		std::vector<std::string> params;
		params.push_back(client->getNickname());
		params.push_back(channelName);
		params.push_back(channel->getTopic());
		reply = createReply(RPL_TOPIC, RPL_TOPIC_STR, params);
		replies.push_back(reply);
	}

	return replies;
}
