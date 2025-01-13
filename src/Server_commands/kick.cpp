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

    Client *main = findClient(message.sender_client_fd);
    std::string nick = main->getNickname();
    std::string kickMsg = "";

    if (message.params.size() < 2)
    {
		std::vector<std::string>	params;
		params.push_back(nick);
		params.push_back("KICK");
        replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, params));
        return replies;
    }

    std::string chName = message.params[1];
    if (!findChannel(chName))
    {
        replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, chName));
        return replies;
    }
    if (!findChannel(chName)->isUserInChannel(nick))
    {
        replies.push_back(createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, chName));
        return replies;
    }
    if (!findChannel(chName)->isUserOperator(nick)){
        replies.push_back(createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, chName));
        return replies;
    }

    if (message.params.size() >= 4)
    {
        for (size_t i = 3; i < message.params.size(); i++)
        {
            kickMsg += message.params[i];
            if (i < message.params.size() - 1)
                kickMsg += " ";
        }
    }
    else
    {
        kickMsg = nick + " has kicked user from channel"; // ??
    }

    std::vector<std::string> targets = parseMessage(message.params[2], ',');
    for (size_t i = 0; i < targets.size(); i++)
    {
        if (!findClient(targets[i]))
        {
            replies.push_back(createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, targets[i]));
            continue;
        }
        if (!findChannel(chName)->isUserInChannel(targets[i]))
        {
			std::vector<std::string>	params;
			params.push_back(targets[i]);
			params.push_back(chName);
            replies.push_back(createReply(ERR_USERNOTINCHANNEL, ERR_USERNOTINCHANNEL_STR, params));
            continue;
        }

        t_message kickNotice;
        kickNotice.prefix = main->getUserPrefix();
        kickNotice.command = "KICK";
        kickNotice.params.push_back(chName);
        kickNotice.params.push_back(targets[i]);
        kickNotice.params.push_back(kickMsg);
        // addChannelToReply addChannelToReply addChannelToReply addChannelToReply addChannelToReply addChannelToReply 
        // kickNotice.target_channels.push_back(findChannel(chName));
        replies.push_back(kickNotice);

        findChannel(chName)->kickUser(targets[i]);
        findChannel(chName)->uninviteUser(targets[i]);
        removeClient(findClient(targets[i])->getSocket());
    }
    if (findChannel(chName)->isEmpty())
    {
        replies.push_back(createReply(RPL_CHANNELREMOVED, RPL_CHANNELREMOVED_STR, chName));
        this->_channels.erase(chName);
    }

    return replies;
}
