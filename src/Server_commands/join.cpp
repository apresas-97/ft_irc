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
	std::vector<t_message> replies;

	Client *client = this->_current_client;

	std::vector<std::string> channels;
    std::vector<std::string> keys;
    std::vector<int> fds;

	bool are_keys = message.params.size() > 2 ? true : false;

	if (message.params.size() < 2) {
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, {client->getNickname(), "JOIN"}));
		return replies;
	}

	std::string channelName = message.params[0];
	// keys = _parseMessage(params[1], ',')

	if (isChannelInServer(channelName)) {
		Channel &channel = _channels[channelName];

		if (channel.getMode('i') && !channel.isUserInvited(client->getUsername())) {
			replies.push_back(createReply(ERR_INVITEONLYCHAN, ERR_INVITEONLYCHAN_STR, channelName));
		}
		if (channel.getMode('l') && channel.h() >= channel.getUserLimit()) {
			replies.push_back(createReply(ERR_CHANNELISFULL, ERR_CHANNELISFULL_STR, channelName));
		}
		// check if too many channels for client
		if (client->getChannelCount() >= client->getChannelLimit()) {
			replies.push_back(createReply(ERR_TOOMANYCHANNELS, ERR_TOOMANYCHANNELS_STR, channelName));
		}
		// check if too many clients in channel
		if (channel.getUserCount() >= channel.getUserLimit()) {
			replies.push_back(createReply(ERR_CHANNELISFULL, ERR_CHANNELISFULL_STR, channelName));
		}
		// check the key if it is required for channel
		if (channel.getMode('k')) {
			if (!are_keys || cl_fd >= (int)message.params.size() - 1 || message.params[cl_fd] != channel.getKey()) {
				replies.push_back(createReply(ERR_BADCHANNELKEY, ERR_BADCHANNELKEY_STR, channelName));
			}
		}

		// Añadir al canal existente
        channel.addUser(*client, cl_fd);
        client->addChannel(channel, channelName);
        sendMessageToChannel(cl_fd, channel, createReply(client->getNickname(), client->getRealname(), client->getHostname(), channelName));
        if (!channel.getTopic().empty()) {
            replies.push_back(createReply(RPL_TOPIC, channel.getTopic(), channelName));
        }
        _rplNamesList(cl_fd, channelName, channel.getClients());
    }
	else
	{
        // Crear un nuevo canal
        if (client->getChannelCount() >= client->getChannelLimit()) {
            replies.push_back(createReply(ERR_TOOMANYCHANNELS, ERR_TOOMANYCHANNELS_STR, channelName));
            return replies;
        }
        if (!_validChannelName(channelName)) {
            replies.push_back(createReply(ERR_BADCHANMASK, ERR_BADCHANMASK_STR, channelName));
            return replies;
        }

        Channel newChannel(channelName);
        newChannel.addUser(*client, true);

        if (!key.empty()) {
            newChannel.setKey(key);
            newChannel.setMode('k', true);
        }

        _channels[channelName] = newChannel;
        client->addChannel(channelName, true);

        sendMessageToChannel(cl_fd, newChannel, createReply(client->getNickname(), client->getRealname(), client->getHostname(), channelName));
        _rplNamesList(cl_fd, channelName, newChannel.getClients());
    }

    return replies;
}

