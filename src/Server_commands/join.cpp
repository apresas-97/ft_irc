#include "Server.hpp"
/*
	Command: JOIN
	Parameters: ( <channel> *( "," <channel> ) [ <key> *( "," <key> ) ] ) / "0"

	The JOIN command is used by a user to request to start listening to the specific channel.

	//// This part I don't understand:
	Servers must be able to parse arguments in the form of a list of target, but SHOULD NOT
	use lists when sending JOIN messages to clients.
	//// End of part ////

	If a JOIN is succesful, the user receives a JOIN message as confirmation and it is then
	sent the channel's topic using RPL_TOPIC and the list of users who are in the channel
	using RPL_NAMREPLY, which must include the user in question.

	JOIN accepts a special argument "0", which is a special request to leave all channels
	the user is currently a member of.
	The server will process this message as if the user had sent a PART command for each
	channel he is a member of.
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

std::vector<t_message>	Server::cmdJoin( t_message & message )
{
	std::cout << "JOIN command called..." << std::endl;
	std::vector<t_message>	replies;
	t_message				reply;

	Client *client = this->_current_client;

	std::vector<std::string> 	channels;
    std::vector<std::string> 	keys;
    std::vector<int> 			fds;

	bool are_keys = message.params.size() > 2 ? true : false;

	if (message.params.size() < 2) 
	{
		std::vector<std::string>	params;
		params.push_back(client->getNickname());
		params.push_back("JOIN");
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, params);
		replies.push_back(reply);
		return replies;
	}

	channels = parseMessage(message.params[0], ',');
	// int channels_n = channels.size();
    if (are_keys)
	{
        keys = parseMessage(message.params[1], ',');
	}

	for (size_t i = 0; i < channels.size(); i++)
	{
		std::string currentChannel = channels[i];
		Channel * channel = _channels[currentChannel];

		if (isChannelInServer(currentChannel))
		{
			// Mode -i (Invite-only channel)
			if (channel->getMode('i') && !channel->isUserInvited(client->getUsername()))
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
			// Mode -o (Operator privileges required)
			if (channel->getMode('o') && !channel->isUserOperator(client->getUsername())) 
			{
				std::vector<std::string> params;
				params.push_back(client->getNickname());
				params.push_back(currentChannel);
				reply = createReply(ERR_NEEDCHANOP, ERR_NEEDCHANOP_STR, params);
				replies.push_back(reply);
				continue;
			}
			// Mode -k (Channel key)
			if (channel->getMode('k'))
			{
				if (!are_keys || i >= keys.size() - 1 || keys[i] != channel->getKey()) 
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
			// Add to the current channel
			channel->addUser(*client, false);
			client->addChannel(*channel, currentChannel);
			fds = channel->getFds("users");
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
			newChannel.addUser(*client, true);

			if (are_keys && i < keys.size())
			{
				newChannel.setKey(keys[i]);
				newChannel.setMode('k', true);
			}

			this->_channels[currentChannel] = &newChannel;
			client->addChannel(newChannel, currentChannel);
			fds = newChannel.getFds("users");
			channel = &newChannel;
		}
		
		// Enviar mensajes de bienvenida al canal
        t_message joinMessage;
        joinMessage.prefix = client->getUserPrefix();
        joinMessage.command = "JOIN";
        joinMessage.params.push_back(currentChannel);
        joinMessage.sender_client_fd = client->getSocket();
		joinMessage.target_client_fds.insert(message.sender_client_fd);
		addChannelToReply(joinMessage, channel);

        if (channel->getTopic() != "")
		{
			std::vector<std::string>	params;
			params.push_back(client->getNickname());
			params.push_back(currentChannel);
			params.push_back(channel->getTopic());
			reply = createReply(RPL_TOPIC, RPL_TOPIC_STR, params);
			replies.push_back(reply);
        }

        replies.push_back(replyList(client, channel, fds));
        fds.clear();
	}

    return replies;
}

