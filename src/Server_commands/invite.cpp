#include "Server.hpp"

std::vector<t_message> Server::cmdInvite(t_message &message) 
{
	std::cout << "INVITE command called..." << std::endl;
	std::vector<t_message>	replies;
	t_message				reply;
	Client *client = this->_current_client;

	if (message.params.size() < 2) 
	{
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname());
		// TODO set reply target fd and sender fd
		replies.push_back(reply);
		return replies;
	}

	std::string targetNickname = message.params[0];
	std::string channelName = message.params[1];

	if (this->_channels.find(channelName) == this->_channels.end()) 
	{
		reply = createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, channelName);
		// TODO set reply target fd and sender fd
		replies.push_back(reply);
		return replies;
	}

	Channel * channel = this->_channels[channelName];

	if (!channel->isUserInChannel(client->getNickname())) 
	{
		reply = createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, channelName);
		// TODO set reply target fd and sender fd	
		replies.push_back(reply);
		return replies;
	}
	// Check how this works and why its different from the past condition
	// if (channel.isMember(targetNickname)) 
	//{
	//	 replies.push_back(createReply(ERR_USERONCHANNEL, ERR_USERONCHANNEL_STR, targetNickname, channelName));
	//	 return replies;
	// }

	if (!channel->isUserOperator(client->getNickname())) 
	{
		reply = createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, channelName);
		// TODO set reply target fd and sender fd
		replies.push_back(reply);
		return replies;
	}

	Client *targetClient = this->findClient(targetNickname);
	if (!targetClient) 
	{
		reply = createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, targetNickname);
		// TODO set reply target fd and sender fd
		replies.push_back(reply);
		return replies;
	}

	channel->addUser(*client, 0);
	t_message inviteMessage;
	inviteMessage.prefix = client->getUserPrefix();
	inviteMessage.command = "INVITE";
	inviteMessage.params.push_back(targetNickname);
	inviteMessage.params.push_back(channelName);
	// targetClient->sendMessage(inviteMessage); I have to check where is this sent

	// OLD RESPONSE
	std::vector<std::string>	args;
	args.push_back(targetNickname);
	args.push_back(channelName);

	// NEW RESPONSE
	// replies.push_back(createReply(RPL_INVITING, RPL_INVITING_STR, targetNickname, channelName));

	reply = createReply(RPL_INVITING, RPL_INVITING_STR, args);
	// TODO set reply target fd and sender fd
	replies.push_back(reply);
	return replies;
}
