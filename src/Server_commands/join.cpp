#include "Server.hpp"

/*
	Command: JOIN
	Parameters: ( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0"

	The JOIN command is used by a user to request to start listening to the specific channel.

	If a JOIN is succesful, the user receives a JOIN message as confirmation and it is then
	sent the channel's topic using RPL_TOPIC and the list of users who are in the channel
	using RPL_NAMREPLY, which must include the user in question.

	JOIN accepts a special argument "0", which is a special request to leave all channels
	the user is currently a member of.
	The server will process this message as if the user had sent a PART command for each
	channel he is a member of.
*/
static std::vector<std::string> parseMessage( const std::string & message, char delimiter ) 
{
	std::vector<std::string> tokens;
	std::string token;
	std::istringstream tokenStream(message);

	while (std::getline(tokenStream, token, delimiter))
		tokens.push_back(token);

	return tokens;
}

std::vector<t_message>	Server::cmdJoin( t_message & message )
{
	std::vector<t_message>	replies;
	t_message				reply;

	Client * client = this->_current_client;
	Channel * channel = NULL;

	std::vector<std::string> 	channel_names;
	std::vector<std::string> 	keys;

	bool are_keys = message.params.size() > 1 ? true : false;

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	if (message.params.size() < 1) 
	{
		std::vector<std::string>	params;
		params.push_back(client->getNickname());
		params.push_back("JOIN");
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, params);
		replies.push_back(reply);
		return replies;
	}
	else if (message.params.size() > 0 && message.params[0] == "0")
	{
		std::vector<Channel *>	_channels = this->_current_client->getChannelsVector();
		size_t		commas = _channels.size() - 1;
		std::string	params;

		for (std::vector<Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		{
			params += (*it)->getName();
			if (commas--)
				params += ",";
		}
		t_message	partCall;

		partCall.command = "part";
		partCall.params.push_back(params);
		partCall.sender_client_fd = this->_current_client->getSocket();
		return (cmdPart(partCall));
	}

	channel_names = parseMessage(message.params[0], ',');
	if (are_keys)
	{
		keys = parseMessage(message.params[1], ',');
	}

	for (size_t i = 0; i < channel_names.size(); i++)
	{
		std::string currentChannel = channel_names[i];

		if (!isValidChannelName(currentChannel))
		{
			reply = createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, currentChannel);
			replies.push_back(reply);
			continue;
		}
		if (isChannelInServer(currentChannel))
		{
			channel = findChannel(currentChannel);
			if (channel->isUserInChannel(client->getNickname()))
				continue;
			// Mode -i (Invite-only channel)
			if (channel->getMode('i') && !channel->isUserInvited(client->getNickname()))
			{
				reply = createReply(ERR_INVITEONLYCHAN, ERR_INVITEONLYCHAN_STR, currentChannel);
				replies.push_back(reply);
				continue;
			}
			// Mode -l (Channel limit)
			if (channel->getMode('l') && channel->getUserCount() >= channel->getUserLimit())
			{
				reply = createReply(ERR_CHANNELISFULL, ERR_CHANNELISFULL_STR, currentChannel);
				replies.push_back(reply);
				continue;
			}
			// Mode -o (Operator privileges required) and Mode -t (Operator privileges required)
			if ((channel->getMode('o') || channel->getMode('t')) && !channel->isUserOperator(client->getNickname())) 
			{
				std::vector<std::string> params;
				params.push_back(client->getNickname());
				params.push_back(currentChannel);
				reply = createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, params);
				replies.push_back(reply);
				continue;
			}
			// Mode -k (Channel key)
			if (channel->getMode('k'))
			{
				if (!are_keys || i > keys.size() - 1 || keys[i] != channel->getKey()) 
				{
					reply = createReply(ERR_BADCHANNELKEY, ERR_BADCHANNELKEY_STR, currentChannel);
					replies.push_back(reply);
					continue;
				}
			}
			if (client->getChannelCount() >= client->getChannelLimit())
			{
				reply = createReply(ERR_TOOMANYCHANNELS, ERR_TOOMANYCHANNELS_STR, currentChannel);
				replies.push_back(reply);
				continue;
			}
			this->addUserToChannel(currentChannel, client, false);
		}
		else
		{
			// Create new channel
			if (client->getChannelCount() >= client->getChannelLimit()) 
			{
				reply = createReply(ERR_TOOMANYCHANNELS, ERR_TOOMANYCHANNELS_STR, currentChannel);
				replies.push_back(reply);	
				return replies;
			}
			Channel newChannel(currentChannel);
			if (are_keys && i < keys.size())
			{
				newChannel.setKey(keys[i]);
				newChannel.setMode('k', true);
			}
			this->addChannel(newChannel, currentChannel);
			this->addUserToChannel(currentChannel, client, true);
			channel = findChannel(currentChannel);
		}
		
		// Enviar mensajes de bienvenida al canal
		t_message joinMessage;
		joinMessage.prefix = ":" + client->getUserPrefix();
		joinMessage.command = "JOIN";
		joinMessage.params.push_back(currentChannel);
		joinMessage.sender_client_fd = client->getSocket();
		joinMessage.target_client_fds.insert(message.sender_client_fd);
		addChannelToReply(joinMessage, channel);
		replies.push_back(joinMessage);

		if (channel->getTopic() != "")
		{
			std::vector<std::string>	params;
			params.push_back(currentChannel);
			params.push_back(channel->getTopic());
			reply = createReply(RPL_TOPIC, RPL_TOPIC_STR, params);
			replies.push_back(reply);
		}

		std::vector<std::string> clientList = channel->getUsersOpClean();
		if (!clientList.empty())
		{
			std::vector<std::string> paramsName;
			paramsName.push_back(currentChannel);
			std::string names;
			for (size_t i = 0; i < clientList.size(); ++i)
				names += clientList[i] + " ";
			paramsName.push_back(names);
			t_message nameReply = createReply(RPL_NAMREPLY, RPL_NAMREPLY_STR, paramsName);
			addChannelToReply(nameReply, channel);
			replies.push_back(nameReply);
			replies.push_back(createReply(RPL_ENDOFNAMES, RPL_ENDOFNAMES_STR, channel->getName()));
		}
	}
	return replies;
}
