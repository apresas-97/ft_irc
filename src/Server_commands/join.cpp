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
std::vector<t_message>	Server::cmdJoin( t_message & message )
{
	std::cout << "JOIN command called..." << std::endl;
	int cl_fd = message.sender_client_fd;
	Client * client = findClient(cl_fd);
	std::vector<t_message> replies;
	bool are_keys = message.params.size() > 2 ? true : false;

	std::string channelName;
	// std::vector<std::string> keys;

	// TODO ... ?

	if (message.params.size() < 2)
	{
		client->sendMessage(/*ERRORNEEDMOREPARAMS*/); // pendant to implement sendMessage
		return ;
	}

	channelName = message.params[0];
	// keys = _parseMessage(params[1], ',')

    if (isChannelInServer(channelName))
	{
		Channel		& channel = _channels[channelName];

		if (channel.getMode('i') && !channel.isUserInvited(client->getUsername()))
		{
			client->sendMessage(/*Err Invite Only*/);
		}
		if (channel.getMode('l') && channel.h() >= channel.getUserLimit())
		{
			client->sendMessage(/*Err Channel is Full*/);
		}
		// check if too many channels for client
		if (client->getChannelCount() >= client->getChannelLimit())
		{
			client->sendMessage(/*Err Too Many Channels*/);
		}
		// check if too many clients in channel
		if (channel.getUserCount() >= channel.getUserLimit())
		{
			client->sendMessage(/*Err Channel is Full*/);
		}
		// check the key if it is required for channel
		if (channel.getMode('k'))
		{
			if (!are_keys || cl_fd >= (int)message.params.size() - 1 || message.params[cl_fd] != channel.getKey())
				client->sendMessage(/*Err Bad Channel Key*/);
		}
		channel.addUser(*client, cl_fd);
		client.addChannel(channelName, false);
		fds = this->_channels[channelName].getClients();
	}
	else
	{
		// TODO
	}
	/*
		sendMessageToChannel(sock, channel, RPL_JOIN(client.getNickname(), client.getRealname(), client.getHostname(), chan_name));
        if (channel.getTopic() != "")
            client.sendMessage(RPL_TOPIC(client.getNickname(), chan_name, channel.getTopic()));
        
        _rplNamesList(sock, chan_name, fds);
        fds.clear();
	*/
	return replies;
}

