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
		   RPL_WHOREPLY					RPL_ENDOFWHO

   Examples:

   WHO *.fi						; List all users who match against
								   "*.fi".

   WHO jto* o					  ; List all users with a match against
								   "jto*" if they are an operator.
*/

#include "Server.hpp"

static std::vector<std::string>	getUserInfo( Client client, std::string server_name, std::string channel_name);
static bool	starts_with(std::string str, std::string prefix);
static bool	ends_with(std::string str, std::string suffix);
static bool	check_prefix(Client client, std::string prefix, std::string server_name);
static bool	check_suffix(Client client, std::string suffix, std::string server_name);

std::vector<t_message> Server::cmdWho( t_message & message )
{
	std::vector<t_message>	replies;
	std::string				name;

	if (message.params.size() > 0)
		name = message.params[0];

	bool	is_all = (name.empty() || name == "0" || name == "*");
	bool	is_channel = (!name.empty() && (name[0] == '#' || name[0] == '&'));

	if (is_all)
	{
		std::cout << "It is wildcard" << std::endl;	/* DEBUG */
		for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
		{
			Client	client = it->second;
			if (client.getMode('i') && client.getSocket() != _current_client->getSocket())
				continue;

			std::vector<std::string>	info = getUserInfo(client, this->_name, "");
			replies.push_back(createReply(352, RPL_WHOREPLY_STR, info));
		}
	}
	else if (name.find('*') != std::string::npos)
	{
		std::cout << "Wildcard contains some information to check" << std::endl; /* DEBUG */
		std::string	prefix = name.substr(0, name.find('*'));
		std::string	suffix = name.substr(name.find('*') + 1);

		for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
		{
			Client	client = it->second;
			if (check_prefix(client, prefix, this->_name) && check_suffix(client, suffix, this->_name))
			{
				std::vector<std::string>	info = getUserInfo(client, this->_name,  "");
				replies.push_back(createReply(352, RPL_WHOREPLY_STR, info));
			}
		}
	}
	else if (is_channel) // Must print information of all users in the channel that are not invisible
	{
		std::cout << "It is channel" << std::endl; /* DEBUG */
		Channel *	channel = this->findChannel(name);

		if (channel && channel->isUserInChannel(this->_current_client->getUsername()))
		{
			std::map<std::string, Client *>	users = channel->getTrueUsers();
			for (std::map<std::string, Client *>::iterator it = users.begin(); it != users.end(); ++it)
			{
				Client	client = *(it->second);
				if (it->second->getMode('i'))
					continue;

				std::vector<std::string>	info = getUserInfo(client, this->_name, channel->getName());
				replies.push_back(createReply(352, RPL_WHOREPLY_STR, info));
			}
		}
	}
	// TODO Control amount of replies and truncate message if amount is too large...
	replies.push_back(createReply(315, RPL_ENDOFWHO_STR, name));
	return replies;
}

static std::vector<std::string>	getUserInfo( Client client, std::string server_name, std::string channel_name)
{
	std::vector<std::string>	user_info;

	// Check if the user is in any channel
	if (client.getChannelCount() > 0)
	{
		if (!channel_name.empty())
			user_info.push_back(channel_name);
		else
		{
			std::vector<Channel *>	channels = client.getChannelsVector();
			for (std::vector<Channel *>::iterator it = channels.begin(); it != channels.end(); ++it)
			{
				user_info.push_back((*it)->getName());
				break ;
			}
		}
	}
	else
	{
		user_info.push_back("*");
	}
	user_info.push_back(client.getNickname());
	user_info.push_back(client.getUsername());
	user_info.push_back(client.getHostname());
	user_info.push_back(server_name);
	if (client.getMode('a'))
	{
		user_info.push_back("A");
//		user_info.push_back(client.getAwayMessage()); // TODO
	}
	else
		user_info.push_back("H");
	user_info.push_back(":" + client.getRealname());

	return user_info;
}

static bool	starts_with(std::string str, std::string prefix) 
{
	// Get lengths of both strings
	size_t str_len = str.size();
	size_t prefix_len = prefix.size();

	// If the prefix is longer than the string, it cannot match
	if (prefix_len > str_len) 
		return false;

	// Compare the prefix with the beginning of the string
	return std::strncmp(str.c_str(), prefix.c_str(), prefix_len) == 0;
}

static bool	ends_with(std::string str, std::string suffix)
{
	// Get lengths of both strings
	size_t str_len = str.size();
	size_t suffix_len = suffix.size();

	// If the suffix is longer than the string, it cannot match
	if (suffix_len > str_len)
		return false;

	// Compare the suffix with the end of the string
	return std::strncmp(str.c_str() + str_len - suffix_len, suffix.c_str(), suffix_len) == 0;
}

	// COMPARE WITH host, server, realname and nickname

static bool	check_prefix(Client client, std::string prefix, std::string server_name)
{
	if (prefix.empty())
		return true;
	if (starts_with(client.getHostname(), prefix) || starts_with(server_name, prefix)  \
		|| starts_with(client.getRealname(), prefix) || starts_with(client.getNickname(), prefix))
		return true;
	return false;
}

static bool	check_suffix(Client client, std::string suffix, std::string server_name)
{
	if (suffix.empty())
		return true;
	if (ends_with(client.getHostname(), suffix) || ends_with(server_name, suffix) \
		|| ends_with(client.getRealname(), suffix) || ends_with(server_name, suffix))
		return true;
	return false;
}

