/*
	Command: WHO
	Parameters: [<name> [<o>]]

	The WHO message is used by a client to generate a query which returns
	a list of information which 'matches' the <name> parameter given by
	the client.

	In the absence of the <name> parameter, all visible (users who aren't 
	invisible (user mode +i) and who don't have a common channel with the 
	requesting client) are listed. The same result can be achieved by using 
	a <name> of "0" or any wildcard which will end up matching every entry 
	possible.

	The <name> passed to WHO is matched against users' host, server, real 
	name and nickname if the channel <name> cannot be found.

	If the "o" parameter is passed only operators are returned according
	to the name mask supplied.

	Numeric Replies:

           ERR_NOSUCHSERVER
           RPL_WHOREPLY                    RPL_ENDOFWHO

   Examples:

   WHO *.fi                        ; List all users who match against
                                   "*.fi".

   WHO jto* o                      ; List all users with a match against
                                   "jto*" if they are an operator.
*/

#include "Server.hpp"

std::vector<t_message> Server::cmdWho( t_message & message )
{
	std::vector<t_message>	replies;
	std::string				name;

	if (message.params.size() > 0)
		name = message.params[0];

	bool	is_wildcard = (name.empty() || name == "0" || name == "*");
	bool	is_channel = (!name.empty() && (name[0] == '#' || name[0] == '&'));

	if (is_wildcard)
	{
		std::cout << "It is wildcard" << std::endl;
		// Look for all the users that don't have mode +i

		// Check if user is in any channel, prefix will be '*' if user is not in any channel, or the first channel name it's in

		for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
		{
			Client	client = it->second;
			if (client.getMode('i') && client.getSocket() != _current_client->getSocket())
				continue;

			std::vector<std::string>	user_info;

			// Check if the user is in any channel
			if (client.getChannelCount() > 0) // TODO Current function is not working
			{
				user_info.push_back("CHANNEL");
				// Prefix will be the first channel name found
			}
			else
			{
				user_info.push_back("*");
				// Prefix should be '*'
			}
			user_info.push_back(client.getNickname());
			user_info.push_back(client.getUsername());
			user_info.push_back(client.getHostname());
			user_info.push_back(this->_name);
			if (client.getMode('a'))
			{
				user_info.push_back("A");
//				user_info.push_back(client.getAwayMessage()); // TODO
			}
			else
				user_info.push_back("H");
			user_info.push_back(":" + client.getRealname());

			std::cout << "PRINTING USER INFO: " << std::endl;
			for (std::vector<std::string>::iterator it = user_info.begin(); it != user_info.end(); ++it)
				std::cout << *it << std::endl;
			std::cout << "END OF PRINTING USER INFO " << std::endl;
			//	<server> 352 <prefix> <user> <username> <hostname> <server> <status> <away> :<realname>
		}
	}
	else if (name.find('*') != std::string::npos)
	{
		std::cout << "Wildcard contains some information to check" << std::endl;
	}
	else if (is_channel)
	{
		std::cout << "It is channel" << std::endl;
		Channel *	channel = this->findChannel(name);

		if (channel && channel->isUserInChannel(this->_current_client->getUsername()))
		{
			std::map<std::string, Client *>	users = channel->getTrueUsers();
			for (std::map<std::string, Client *>::iterator it = users.begin(); it != users.end(); ++it)
			{
				if (!it->second->getMode('i')) // Add user to who list...
				{
					std::cout << "Adding user: " << it->second->getNickname() << std::endl;
				}
			}
		}
		else // Channel not found or User not in channel
		{
			std::cout << "Unable to process who command" << std::endl;
		}
	}
	// Send end of who
	return replies;
}

