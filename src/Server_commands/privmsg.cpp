/*

    0. Management
    1. Validation
        Does it have a recipient? Args < 1
        Does it have enough text? Args < 2
    3. Extract targets
    4. Prepare the text to send
        Iterate over the text and add a space to each parameter of tmessage->params
    5. Iterate over the targets
        5.a. If # or & (Channels)
            5.a.a. The channel exists
            5.a.b. The channel does NOT exist
        5.b. Bot
        5.c. The target nickname exists
            5.c.a. The nickname does not exist

*/

#include "Server.hpp"

static std::vector<std::string> parseMessage(const std::string &message, char delimiter) 
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(message);

    while (std::getline(tokenStream, token, delimiter))
        tokens.push_back(token);

    return tokens;
}

std::vector<t_message> Server::cmdPrivMsg(t_message &message) 
{
    std::cout << "PRIVMSG command called..." << std::endl;
    std::vector<t_message> replies;
    Client *client = _current_client;
    std::vector<std::string> targets;
    std::string textToSend;

    // Validate that there are enough parameters
    if (message.params.size() < 2)
    {
//        replies.push_back(createReply(ERR_NORECIPIENT, ERR_NORECIPIENT_STR, {client->getNickname(), "PRIVMSG"})); // incorrect call
        return replies;
    }

    // Extract the recipients
    targets = parseMessage(message.params[0], ',');

    // Validate that there is text to send
    if (message.params.size() < 2)
    {
        replies.push_back(createReply(ERR_NOTEXTTOSEND, ERR_NOTEXTTOSEND_STR, client->getNickname()));
        return replies;
    }

    // Construct the full message text
    textToSend = message.params[1];
    for (size_t i = 2; i < message.params.size(); i++)
        textToSend += " " + message.params[i];

    // Iterate over each recipient
    for (size_t i = 0; i < targets.size(); i++)
    {
        std::string &target = targets[i];

        // It's a channel
        if (target[0] == '#' || target[0] == '&') 
        {
            if (_channels.find(target) != _channels.end()) 
            {
                Channel &channel = _channels[target];

                if (!channel.isUserInChannel(client->getNickname())) 
				{
 //                   replies.push_back(createReply(ERR_CANNOTSENDTOCHAN, ERR_CANNOTSENDTOCHAN_STR, {client->getNickname(), target})); // incorrect call
                    continue;
                }

                // Send the message to the channel
                t_message channelMessage;
                channelMessage.prefix = client->getUserPrefix();
                channelMessage.command = "PRIVMSG";
                channelMessage.params.push_back(target);
                channelMessage.params.push_back(textToSend);
                channelMessage.sender_client_fd = client->getSocket();
                channelMessage.target_channels.push_back(&channel);
                // Add channel to my replies and remember to add the channel I want to send the message to into the vector inside t_message called target_channels;
                // channel.messageToChannel(channelMessage, client->getNickname());
            }
            else
            {
//                replies.push_back(createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, {client->getNickname(), target})); // incorrect call
            }
        } 
        // Send the message to a specific client
        else if (Client *targetClient = findClient(target)) 
        {
            t_message privateMessage;
            privateMessage.prefix = client->getUserPrefix();
            privateMessage.command = "PRIVMSG";
            privateMessage.params.push_back(target);
            privateMessage.params.push_back(textToSend);
            privateMessage.target_client_fd = targetClient->getSocket();
            // Scrape target user and find it's fd using find client or something
            // Set target fd of the message into that target fd and use the function sendMessage located in server_loop? maybe
            // targetClient->sendMessage(privateMessage);
        } 
        else 
        {
//            replies.push_back(createReply(ERR_NOSUCHNICK, ERR_NOSUCHNICK_STR, {client->getNickname(), target})); // incorrect call
        }
    }

    return replies;
}

