#include "Server.hpp"

std::vector<t_message> Server::cmdInvite(t_message &message) 
{
	std::cout << "INVITE command called..." << std::endl;
	std::vector<t_message>	replies;
	t_message				reply;
	Client *client = this->_current_client;

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	if (message.params.size() < 2)
	{
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname());
		replies.push_back(reply);
		return replies;
	}

	std::string & targetNickname = message.params[0];
	std::string & channelName = message.params[1];

	if (this->isChannelInServer(channelName) == false) 
	{
		reply = createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, channelName);
		replies.push_back(reply);
		return replies;
	}

	Channel * channel = this->findChannel(channelName);

	if (!channel->isUserInChannel(client->getNickname()))
	{
		reply = createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, channelName);
		replies.push_back(reply);
		return replies;
	}
	// return replies;
	// Check how this works and why its different from the past condition
	// if (channel->isMember(targetNickname)) 
	//{
	//	 replies.push_back(createReply(ERR_USERONCHANNEL, ERR_USERONCHANNEL_STR, targetNickname, channelName));
	//	 return replies;
	// }

	if (!channel->isUserOperator(client->getNickname()))
	{
		reply = createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, channelName);
		replies.push_back(reply);
		return replies;
	}

	if (!this->isUserInServer(targetNickname))
	{
		reply = createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, targetNickname);
		replies.push_back(reply);
		return replies;
	}

	Client * targetClient = this->findClient(targetNickname);
	channel->addUser(client, 0);
	t_message inviteMessage;
	inviteMessage.prefix = client->getUserPrefix();
	inviteMessage.command = "INVITE";
	inviteMessage.params.push_back(targetNickname);
	inviteMessage.params.push_back(channelName);
	inviteMessage.target_client_fds.insert(targetClient->getSocket());
	// I've seen in some servers that the acknowledgement message is sent ALSO to the rest of
	// operators of the channel. It's not our priority right now.

	std::vector<std::string>	params;
	params.push_back(channelName);
	params.push_back(targetNickname);
	reply = createReply(RPL_INVITING, RPL_INVITING_STR, params);
	replies.push_back(reply);
	return replies;
}
