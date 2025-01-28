#include "Server.hpp"
#include <iomanip>

/* returns a numeric reply number in its string format with 3 width and padding 0 */
static std::string formatNumber( int number )
{
	std::ostringstream	oss;
	oss << std::setw(3) << std::setfill('0') << number;
	return oss.str();
}

// Currently unused
static std::string getTimestamp( void )
{
	std::string timestamp = "@time=";
	time_t t = time(0);
	tm *now = localtime(&t);

	std::ostringstream	oss;
	oss << (now->tm_year + 1900) << '-'
		<< std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '-'
		<< std::setw(2) << std::setfill('0') << now->tm_mday << 'T'
		<< std::setw(2) << std::setfill('0') << now->tm_hour << ':'
		<< std::setw(2) << std::setfill('0') << now->tm_min << ':'
		<< std::setw(2) << std::setfill('0') << now->tm_sec << "Z";

	timestamp += oss.str();
	return timestamp;
}

// For replies that have no specific parameters like <channel>, <nickname>, etc.
t_message Server::createReply( int number, const std::string message )
{
	t_message reply;
	// reply.timestamp = getTimestamp(); // optional
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
	// std::string timestamp = getTimestamp(); // optional
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
	// std::string timestamp = getTimestamp(); // optional
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
	// std::string	nickname = this->_current_client->getNickname();
	// if (nickname.empty())
		// nickname = "*"; // I don't know what to do in this case rn
	reply.params.push_back(nickname);
	std::string param = corrected_param; // TODO: Figure out the ":" part
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

	// Create the response with the list of users
	// RPL_NAMREPLY is the IRC response code for the name list

//    t_message reply = createReply(RPL_NAMREPLY, RPL_NAMREPLY_STR, {main, channel->getName(), userlist}); // TODO incorrect call
	t_message	reply; // NEEDED IN ORDER TO AVOID PROGRAM SHITTING ITSELF BECAUSE PREVIOUS CALL IS WRONG
    return reply;
}
