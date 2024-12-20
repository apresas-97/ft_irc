#include "Server.hpp"

std::vector<t_message> Server::cmdInvite(t_message &message) {
    std::cout << "INVITE command called..." << std::endl;
    std::vector<t_message> replies;
    Client *client = this->_current_client;

    if (message.params.size() < 2) {
        replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname()));
        return replies;
    }

    std::string targetNickname = message.params[0];
    std::string channelName = message.params[1];

    if (this->_channels.find(channelName) == this->_channels.end()) {
        replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, channelName));
        return replies;
    }

    Channel &channel = this->_channels[channelName];

    if (!channel.isUserInChannel(client->getNickname())) {
        replies.push_back(createReply(ERR_NOTONCHANNEL, ERR_NOTONCHANNEL_STR, channelName));
        return replies;
    }
    // Check how this works and why its different from the past condition
    // if (channel.isMember(targetNickname)) {
    //     replies.push_back(createReply(ERR_USERONCHANNEL, ERR_USERONCHANNEL_STR, targetNickname, channelName));
    //     return replies;
    // }

    if (!channel.isUserOperator(client->getNickname())) {
        replies.push_back(createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, channelName));
        return replies;
    }

    Client *targetClient = this->findClient(targetNickname);
    if (!targetClient) {
        replies.push_back(createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, targetNickname));
        return replies;
    }

    channel.addUser(*client, 0);
    t_message inviteMessage;
    inviteMessage.prefix = client->getUserIdentifier();
    inviteMessage.command = "INVITE";
    inviteMessage.params.push_back(targetNickname);
    inviteMessage.params.push_back(channelName);
    // targetClient->sendMessage(inviteMessage); I have to check where is this sent

    replies.push_back(createReply(RPL_INVITING, RPL_INVITING_STR, targetNickname, channelName));

    return replies;
}
