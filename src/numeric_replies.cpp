#include "Server.hpp"
#include <iomanip>

/* returns a numeric reply number in its string format with 3 width and padding 0 */
static std::string formatNumber( int number )
{
	std::ostringstream	oss;
	oss << std::setw(3) << std::setfill('0') << number;
	return oss.str();
}

// For replies that have no specific parameters like <channel>, <nickname>, etc.
t_message Server::createReply( int number, const std::string message )
{
	t_message reply;
	reply.prefix = ":" + this->getName();
	reply.command = formatNumber(number);
	std::string	nickname = this->_current_client->getNickname();
	if (nickname.empty())
		nickname = "*";
	reply.params.push_back(nickname);
	reply.params.push_back(message);
	reply.target_client_fds.insert(this->_current_client->getSocket());
	return reply;
}

/*
For replies that have 1 parameter
param will replace the placeholder in the message
*/
t_message Server::createReply( int number, const std::string message, const std::string & param )
{
	t_message reply;
	reply.prefix = ":" + this->getName();
	reply.command = formatNumber(number);

	std::string	nickname = this->_current_client->getNickname();
	if (nickname.empty())
		nickname = "*";
	reply.params.push_back(nickname);

	std::string msg = message;

	// Replace the <placeholder> in the message with the param
	size_t start = msg.find('<');
	size_t end = msg.find('>');
	if (start != std::string::npos && end != std::string::npos) 
		msg.replace(start, end - start + 1, param);
	reply.params.push_back(msg);
	reply.target_client_fds.insert(this->_current_client->getSocket());
	return reply;
}

/*
For replies that require multiple parameters
Each member of the param vector will replace a placeholder in the message in order from left to right
*/
t_message Server::createReply( int number, const std::string message, const std::vector<std::string> & params ) 
{
	t_message reply;
	reply.prefix = ":" + this->getName();
	reply.command = formatNumber(number);
	std::string	nickname = this->_current_client->getNickname();
	if (nickname.empty())
		nickname = "*";
	reply.params.push_back(nickname);
	std::string msg = message;

	// Replace the <placeholders> in the message with the param, in order of appearance
	size_t start = msg.find('<');
	size_t end = msg.find('>');
	for (size_t i = 0; i < params.size(); i++) 
	{
		if (start != std::string::npos && end != std::string::npos) 
			msg.replace(start, end - start + 1, params[i]);
		start = msg.find('<');
		end = msg.find('>');
	}
	reply.params.push_back(msg);
	reply.target_client_fds.insert(this->_current_client->getSocket());
	return reply;
}

// Provisional, this is the createReply for acknowledge messages to be sent back to the sender
t_message Server::createReply( t_message & message, std::string corrected_param, std::string nickname ) 
{
	t_message	reply;
	reply.prefix = ":" + nickname;
	reply.command = message.command;
	reply.params.push_back(nickname);
	std::string param = corrected_param;
	reply.params.push_back(param);
	reply.target_client_fds.insert(this->_current_client->getSocket());
	return reply;
}

t_message Server::replyList(Client *client, Channel *channel, std::vector<int>& fds)
{
    std::string userlist("");
    std::string main = client->getNickname();

	// Generate the list of channel users with the appropriate format
    for (size_t	i = 0; i < fds.size(); i++) 
    {
        Client *currentClient = findClient(fds[i]);
        std::string currentNick = currentClient->getNickname();

        // Check if the user is an administrator (with operator in the channel)
        if (channel->isUserOperator(currentNick)) 
        {
            userlist += "@" + currentNick + " ";
        }
        else
        {
            userlist += currentNick + " ";
        }
    }

	t_message	reply; // NEEDED IN ORDER TO AVOID PROGRAM SHITTING ITSELF BECAUSE PREVIOUS CALL IS WRONG
    return reply;
}
