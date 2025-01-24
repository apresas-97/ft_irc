#include "Server.hpp"
#include <sstream>
#include <climits>

typedef struct s_flag
{
	char	c;
	bool	is_operator;
	char	active_operator;
	bool	requires_param;
	bool	has_param;
	size_t	param_index;
	std::string	param;
}				t_flag;

typedef struct s_first_appearances
{
	bool	i;
	bool	t;
	bool	k;
	bool	l;
}				t_first_appearances;

typedef struct s_first_appearances_user
{
	// bool	a; // Not used
	bool	i;
	bool	w;
	bool	o;
	bool	O;
	bool	r;
	bool	s;
}				t_first_appearances_user;

static char	findLastOperator( const std::string & str )
{
	for (int i = str.size() - 1; i >= 0; --i)
	{
		if (str[i] == '+' || str[i] == '-')
			return str[i];
	}
	return '\0';
}

static long	getLimit( const std::string & str )
{
	std::istringstream iss(str);
	long limit;
	iss >> limit;
	if (iss.fail())
		return -1;
	if (limit > INT_MAX || limit <= 0)
		return -1;
	return limit;
}

// // Converts a string to a long, returns -1 if the string is not valid
// static long limitStrToLong( const std::string & str )
// {
// 	std::istringstream iss(str);

// 	long limit;
// 	iss >> limit;
// 	if (iss.fail())
// 		return -1;
// 	if (limit > 2147483647 || limit <= 0)
// 		return -1; // Maybe
// 	return limit;
// }

static bool	isKeyValid( const std::string & key )
{
	if (key.size() < 1 || key.size() > 23)
		return false;
	for (size_t i = 0; i < key.size(); i++) 
	{
		if (key[i] == '\0' || key[i] == '\6' || key[i] == '\t' || key[i] == '\n' || key[i] == '\v' || key[i] == '\f' || key[i] == '\r' || key[i] == ' ')
			return false;
	}
	return true;
}

/*
Command: MODE

This is the main MODE function, it will stem into either cmdModeChannel or cmdModeUser, which
have different implementations for channel and user modes, respectively.
*/
std::vector<t_message> Server::cmdMode( t_message & message ) 
{
	std::cout << "MODE command called..." << std::endl;
	std::vector<t_message>	replies;
	Client * client = this->_current_client;

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	if (message.params.size() < 1)
	{
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname()));
		return replies;
	}

	std::cout << "Checking if the given parameter is a channel or a user..." << std::endl;
	std::cout << "Given parameter: " << message.params[0] << std::endl;
	if (isUserInServer(message.params[0]) == false)
		replies = cmdModeChannel(message);
	else
		replies = cmdModeUser(message);

	return replies;
}

/*
(User mode message)
Command: MODE
Params: <nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )

The User MODE message must only be accepted if both the sender and the
nickname given as a parameter are the same.
If no other parameters are given, then the server will return the current
settings for the nick.

Additional parameters can be given to change the nick's modes.

Notes:
The flag 'a' MUST NOT be toggled using this command. Only with the AWAY command.

Attempts to set +o or +O SHOULD be ignored. But -o and -O are allowed.

Attempts to set -r SHOULD be ignored. But +r is allowed.
Though +r is usually set by the server.

Numeric replies:
- ERR_NEEDMOREPARAMS : If no parameters are given (This was already handled in the main MODE command)
- ERR_USERSDONTMATCH : If the given parameter is not the nickname of the sender
- ERR_UMODEUNKNOWNFLAG : If an unknown flag is given (Doesn't negate the known flags in the parameters)
- RPL_UMODEIS : Reply with the current modes of the user (if no additional parameters are given)
*/
std::vector<t_message>	Server::cmdModeUser( t_message & message )
{
	std::cout << "MODE user command called..." << std::endl;
	std::vector<t_message>	replies;
	Client * client = this->_current_client;

	std::string previous_modes = client->getModeString();

	std::cout << "Param[0]: \"" << message.params[0] << "\"" << std::endl;
	std::cout << "Nickname: \"" << client->getNickname() << "\"" << std::endl;
	if (message.params[0] != client->getNickname())
	{
		replies.push_back(createReply(ERR_USERSDONTMATCH, ERR_USERSDONTMATCH_STR));
		return replies;
	}

	if (message.params.size() == 1) 
	{
		replies.push_back(createReply(RPL_UMODEIS, RPL_UMODEIS_STR, client->getModeString()));
		return replies;
	}

	std::string & parameter = message.params[1];
	std::string corrected_mode_param;
	std::vector<t_flag> flags;
	std::map<char, bool> isFirst;
	isFirst['i'] = true;
	isFirst['w'] = true;
	isFirst['o'] = true;
	isFirst['O'] = true;
	isFirst['r'] = true;
	isFirst['s'] = true;
	for (size_t i = 0; i < parameter.size(); ++i)
	{
		t_flag flag;
		flag.c = parameter[i];
		flag.is_operator = parameter[i] == '+' || parameter[i] == '-';
		if (flag.is_operator)
			flag.active_operator = flag.c;
		else if (i == 0)
			flag.active_operator = '+';
		else 
			flag.active_operator = flags.back().active_operator;
		flag.requires_param = false; // user modes don't have parameters
		flag.has_param = false;
		flag.param = "";
		flags.push_back(flag);
	}
	bool unknownmode_err_set = false;
	bool user_mode = false;
	bool set_mode = false;
	std::cout << "Parameter size = " << parameter.size() << std::endl;
	for (size_t i = 0; i < parameter.size(); ++i)
	{
		std::cout << "i = " << i << std::endl;
		char flag = flags[i].c;
		user_mode = client->hasMode(flag);
		set_mode = (flags[i].active_operator == '+');
		if (flags[i].is_operator)
			continue;
		if (std::string(USER_MODES).find(flag) != std::string::npos)
		{
			std::cout << "Flag: " << flag << std::endl;
			std::cout << "has mode '" << flag << "'?: " << user_mode << std::endl;
			std::cout << "set mode: " << set_mode << std::endl;
			std::cout << "Will acknowledge? " << (set_mode != user_mode) << std::endl;
			if (flag == 'a')
				continue;
			if (flag == 'o' || flag == 'O')
				if (flags[i].active_operator == '+') continue;
			if (flag == 'r')
				if (flags[i].active_operator == '-') continue;
			if (isFirst[flag] == false) { continue; } isFirst[flag] = false;
			if (set_mode != user_mode)
			{
				client->setMode(flag, set_mode);
				if (findLastOperator(corrected_mode_param) != flags[i].active_operator)
					corrected_mode_param += flags[i].active_operator;
				corrected_mode_param += flag;
			}
		}
		else if (unknownmode_err_set == false)
		{
			unknownmode_err_set = true;
			replies.push_back(createReply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR));
		}
	}

	std::cout << "Received mode param: " << parameter << std::endl;
	std::cout << "Corrected mode param: " << corrected_mode_param << std::endl;

	std::cout << "User MODES before: " << previous_modes << std::endl;
	std::cout << "User MODES after: " << client->getModeString() << std::endl;
	
	if (corrected_mode_param.size() == 0)
		return replies;

	t_message acknowledgement;
	acknowledgement.prefix = ":" + client->getUserPrefix();
	acknowledgement.command = "MODE";
	acknowledgement.params.push_back(client->getNickname());
	acknowledgement.params.push_back(corrected_mode_param);
	acknowledgement.sender_client_fd = client->getSocket();
	acknowledgement.target_client_fds.insert(client->getSocket());
	replies.push_back(acknowledgement);

	return replies;
}

/*
(Channel mode message)
Command: MODE
Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )

The channel MODE message is used to query and change the characteristics of a channel.

Flags:

- i : Toggle invite-only channel. If set, only invited users can join the channel.

- t : Toggle topic settable by channel operator only.

- k : add/remove a channel key (password) that will be required to join the channel.
	Requires a parameter both to ADD and REMOVE the key.

- o : give/take channel operator privileges to a user in the channel
	Requires a parameter, the nickname of the user to give/take operator privileges.

- l : set the user limit to channel
	Requires a parameter, the limit of users in the channel. i.e. +l 10

There is a limit of 3 changes per command for modes that take a parameter.

Numeric replies:
- ERR_NEEDMOREPARAMS : If no parameters are given (This was already handled in the main MODE command)
- ERR_KEYSET : If key is already set (Unsure of the exact behavior)
- ERR_NOCHANMODES : If channel doesn't support modes (Unsure of the exact behavior)
- ERR_CHANOPRIVSNEEDED : If the client is trying to set a mode without being a channel operator
- ERR_USERNOTINCHANNEL : If the client is trying to set a mode on a user that is not in the channel
- ERR_UNKNOWNMODE : If an unknown mode is given (Doesn't negate the known modes in the parameters, I think)

- RPL_CHANNELMODEIS : If no additional parameters are given, the server will return the current settings for the channel
- RPL_BANLIST : If the client is querying the ban list
- RPL_ENDOFBANLIST : Reply to indicate the end of the ban list
- RPL_EXCEPTLIST : If the client is querying the exception list
- RPL_ENDOFEXCEPTLIST : Reply to indicate the end of the exception list
- RPL_INVITELIST : If the client is querying the invite list
- RPL_ENDOFINVITELIST : Reply to indicate the end of the invite list
- RPL_UNIQOPIS : If the client is querying the unique operator list
*/
std::vector<t_message>	Server::cmdModeChannel( t_message & message )
{
	std::cout << "MODE channel command called..." << std::endl;
	std::vector<t_message>	replies;
	Client * client = this->_current_client;
	Channel * channel = findChannel(message.params[0]);

	if (channel == NULL)
	{
		replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, message.params[0]));
		return replies;
	}

	if (message.params.size() == 1)
	{
		std::cout << "No additional parameters given, RPL_CHANNELMODEIS" << std::endl;
		// RPL_CHANNELMODEIS
		std::vector<std::string> params;
		params.push_back(channel->getName());
		params.push_back(channel->getModesString());
		std::cout << "Channel modes: \"" << channel->getModesString() << "\"" << std::endl;
		// If the sender IS NOT a channel member, the mode reply will not show the parameters
		// i.e. >> :prefix MODE #chan +itkl
		// If the sender IS a channel member, the mode reply will show the parameters
		// i.e. >> :prefix MODE #chan +itkl password 10
		if (channel->isUserInChannel(client->getNickname()) == false)
			params.push_back("");
		else
			params.push_back(channel->getModesParameters());

		replies.push_back(createReply(RPL_CHANNELMODEIS, RPL_CHANNELMODEIS_STR, params));
		return replies;
	}

	// if (channel->isUserInChannel(client->getNickname()) == false)
	// {
	// 	std::vector<std::string> params;
	// 	params.push_back(client->getNickname());
	// 	params.push_back(channel->getName());
	// 	replies.push_back(createReply(ERR_USERNOTINCHANNEL, ERR_USERNOTINCHANNEL_STR, params));
	// 	return replies;
	// }

	if (channel->isUserOperator(client->getNickname()) == false)
	{
		replies.push_back(createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, channel->getName()));
		return replies;
	}

	std::string & parameter = message.params[1];
	std::string corrected_mode_param;
	std::vector<std::string> used_params;
	t_first_appearances first_appearances = { true, true, true, true };
	std::vector<t_flag> flags;
	std::cout << "Before starting, message.params.size() = " << message.params.size() << std::endl;
	std::cout << "Initialising t_flag vector" << std::endl;
	size_t param_index = 2;
	for (size_t i = 0; i < parameter.size(); ++i)
	{
		t_flag flag;
		flag.c = parameter[i];
		flag.is_operator = parameter[i] == '+' || parameter[i] == '-';
		// flag.is_flag = std::string(CHANNEL_MODES).find(flag.c) != std::string::npos;
		if (flag.is_operator)
			flag.active_operator = flag.c;
		else if (i == 0)
			flag.active_operator = '+';
		else 
			flag.active_operator = flags.back().active_operator;
		flag.requires_param = (flag.c == 'k' || flag.c == 'o' || (flag.c == 'l' && flag.active_operator == '+'));
		std::cout << "Requires_param?" << flag.requires_param << std::endl;
		flag.param_index = flag.requires_param ? param_index++ : 0;
		std::cout << "Param_index = " << flag.param_index << std::endl;
		flag.has_param = message.params.size() > flag.param_index;
		std::cout << "Has_param? = " << flag.has_param << std::endl;
		flag.param = flag.has_param ? message.params[flag.param_index] : "";
		flags.push_back(flag);
	}
	std::cout << "----------------------------------------------------" << std::endl;
	std::cout << "Going through each flag" << std::endl;
	for (size_t i = 0; i < parameter.size(); ++i)
	{
		std::cout << "Checking flags[" << i << "] = \'" << flags[i].c << "\'" << std::endl;
		if (flags[i].is_operator)
		{
			std::cout << "Is operator, continue;" << std::endl;
			continue;
		}
		std::cout << "Requires parameters? " << flags[i].requires_param << std::endl;
		std::cout << "Has parameter? " << flags[i].has_param << std::endl;
		if (flags[i].requires_param && flags[i].has_param == false)
		{
			std::cout << "requires_param and does not have param" << std::endl;
			if (flags[i].c == 'l') // I don't know, I don't like this but it is what it is
				replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, message.command));
			std::cout << "continue;" << std::endl;
			continue;
		}
		std::cout << "Switch statement for \'" << flags[i].c << "\'" << std::endl;
		switch (flags[i].c)
		{
			case 'i':
			{
				// std::cout << "case i" << std::endl;
				if (first_appearances.i == false) continue;
				first_appearances.i = false;
				channel->setMode(flags[i].c, flags[i].active_operator == '+');
				if (findLastOperator(corrected_mode_param) != flags[i].active_operator)
					corrected_mode_param += flags[i].active_operator;
				corrected_mode_param += "i";
				break;
			}
			case 't':
			{
				// std::cout << "case t" << std::endl;
				if (first_appearances.t == false) continue;
				first_appearances.t = false;
				channel->setMode(flags[i].c, flags[i].active_operator == '+');
				if (findLastOperator(corrected_mode_param) != flags[i].active_operator)
					corrected_mode_param += flags[i].active_operator;
				corrected_mode_param += "t";
				break;
			}
			case 'k':
			{
				// std::cout << "case k" << std::endl;
				if (first_appearances.k == false) continue;
				first_appearances.k = false;
				std::cout << "First 'k' appearance" << std::endl;
				std::cout << "Active operator = \'" << flags[i].active_operator << "\'" << std::endl;
				if (flags[i].active_operator == '+')
				{
					if (channel->getMode('k') == true)
					{
						std::cout << "Channel already has a key, reply ERR_KEYSET" << std::endl;
						replies.push_back(createReply(ERR_KEYSET, ERR_KEYSET_STR, channel->getName()));
						continue;
					}
					std::string key = flags[i].param;
					if (key.size() > 23)
						key.resize(23); // TODO Use macro for MAX_CHANNEL_KEY_LENGTH or something maybe?
					if (isKeyValid(key) == false) continue;
					channel->setKey(key);
					channel->setMode(flags[i].c, true);
					used_params.push_back(key);
					if (findLastOperator(corrected_mode_param) != flags[i].active_operator)
						corrected_mode_param += flags[i].active_operator;
					corrected_mode_param += "k";
				}
				else if (flags[i].active_operator == '-')
				{
					if (channel->getMode('k') == false)
						continue;
					std::string key = flags[i].param;
					if (key.size() > 23)
						key.resize(23);
					if (key != channel->getKey())
						continue;
					channel->setKey("");
					channel->setMode(flags[i].c, false);
					used_params.push_back(key);
					if (findLastOperator(corrected_mode_param) != flags[i].active_operator)
						corrected_mode_param += flags[i].active_operator;
					corrected_mode_param += "k";
				}
				break;
			}
			case 'o':
			{
				// std::cout << "case o" << std::endl;
				if (channel->isUserInChannel(flags[i].param) == false)
				{
					std::vector<std::string> params;
					params.push_back(flags[i].param);
					params.push_back(channel->getName());
					replies.push_back(createReply(ERR_USERNOTINCHANNEL, ERR_USERNOTINCHANNEL_STR, params));
					continue;
				}
				if (flags[i].active_operator == '+')
					channel->promoteUser(flags[i].param);
				else // '-'
					channel->demoteUser(flags[i].param);
				used_params.push_back(flags[i].param);
				if (findLastOperator(corrected_mode_param) != flags[i].active_operator)
					corrected_mode_param += flags[i].active_operator;
				corrected_mode_param += "o";
				break;
			}
			case 'l':
			{
				// std::cout << "case l" << std::endl;
				if (first_appearances.l == false) continue;
				first_appearances.l = false;
				long limit = 0;
				if (flags[i].active_operator == '+')
				{
					limit = getLimit(flags[i].param);
					if (limit == -1)
						continue;
					used_params.push_back(flags[i].param);
				}
				channel->setMode(flags[i].c, flags[i].active_operator == '+');
				channel->setUserLimit(limit);
				if (findLastOperator(corrected_mode_param) != flags[i].active_operator)
					corrected_mode_param += flags[i].active_operator;
				corrected_mode_param += "l";
				break;
			}
			default:
			{
				// std::cout << "case default" << std::endl;
				std::vector<std::string> params;
				params.push_back(std::string(1, flags[i].c));
				params.push_back(channel->getName());
				replies.push_back(createReply(ERR_UNKNOWNMODE, ERR_UNKNOWNMODE_STR, params));
			}
		}
		std::cout << "----------------------------------------------------" << std::endl;
	}

	std::cout << "Received mode param: " << parameter << std::endl;
	std::cout << "Corrected mode param: " << corrected_mode_param << std::endl;

	if (!corrected_mode_param.empty() && used_params.empty())
		corrected_mode_param += " "; // To make it match exactly with the rawlogs of other IRC servers
	if (corrected_mode_param.empty())
		return replies;

	t_message acknowldegement;
	acknowldegement.prefix = ":" + client->getUserPrefix();
	acknowldegement.command = "MODE";
	acknowldegement.params.push_back(channel->getName());
	acknowldegement.params.push_back(corrected_mode_param);
	for (std::vector<std::string>::iterator it = used_params.begin(); it != used_params.end(); ++it)
		acknowldegement.params.push_back(*it);
	acknowldegement.target_client_fds.insert(client->getSocket());
	replies.push_back(acknowldegement);

	t_message broadcast_message = acknowldegement;
	broadcast_message.target_client_fds.clear();
	addChannelToReplyExcept(broadcast_message, channel);
	replies.push_back(broadcast_message);

	std::cout << "MODE CHANNEL FUNCTION OVER" << std::endl;
	return replies;
}

// Previous Version, just in case
// std::vector<t_message>	Server::cmdModeChannel( t_message & message )
// {
// 	std::vector<t_message>	replies;
// 	Client * client = this->_current_client;
// 	Channel * channel = findChannel(message.params[0]);

// 	// If the given parameter is not a channel in this server
// 	if (channel == NULL)
// 	{
// 		replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, message.params[0]));
// 		return replies;
// 	}

// 	// If the sender of this message is not in the channel
// 	if (channel->isUserInChannel(client->getNickname()) == false)
// 	{
// 		std::vector<std::string> params;
// 		params.push_back(client->getNickname());
// 		params.push_back(channel->getName());
// 		replies.push_back(createReply(ERR_USERNOTINCHANNEL, ERR_USERNOTINCHANNEL_STR, params));
// 		return replies;
// 	}

// 	// First of all, if the message has no parameters after the channel name, it should trigger the RPL_CHANNELMODEIS reply
// 	if (message.params.size() == 1)
// 	{
// 		// TODO
// 		std::vector<std::string> params;
// 		params.push_back(channel->getName());
// 		// params.push_back(channel->getModeString());
// 		replies.push_back(createReply(RPL_CHANNELMODEIS, RPL_CHANNELMODEIS_STR, params));
// 		return replies;
// 	}

// 	// If the sender is not a channel operator
// 	if (channel->isUserOperator(client->getNickname()) == false) // Something like this, TODO check this
// 	{
// 		replies.push_back(createReply(ERR_CHANOPRIVSNEEDED, ERR_CHANOPRIVSNEEDED_STR, channel->getName()));
// 		return replies;
// 	}

// 	std::vector<std::string> ack_params; // Parameters of the acknowledgment message
// 	ack_params.push_back(message.params[0]);
// 	std::string mode_param = message.params[1]; // The mode parameter string sent by the client
// 	std::string corrected_mode_param; // The corrected mode parameter string for the acknowledgment message

// 	std::vector<t_mode_flag> flags;
// 	t_first_appearances first_appearances = { true, true, true, true };
// 	bool operator_to_be_added = true; // Maybe
// 	char last_inserted_operator = '\0'; // The operator that is currently active
// 	size_t j = 2; // The index of the parameter we are currently working with
// 	for (size_t i = 0; i < mode_param.size(); ++i)
// 	{
// 		t_mode_flag flag;
// 		flag.c = mode_param[i];
// 		flag.is_operator = mode_param[i] == '+' || mode_param[i] == '-';
// 		flag.is_flag = std::string(CHANNEL_MODES).find(flag.c) != std::string::npos;
// 		if (i == 0)
// 		{
// 			if (flag.c == '-')
// 				flag.active_operator = '-';
// 			else
// 				flag.active_operator = '+';
// 		}
// 		else if (flag.is_operator)
// 			flag.active_operator = flag.c;
// 		else
// 			flag.active_operator = flags.back().active_operator;
// 		flag.requires_param = (mode_param[i] == 'k' || mode_param[i] == 'o' || (mode_param[i] == 'l' && flag.active_operator == '+'));
// 		flag.param_index = flag.requires_param ? j++ : 0;
// 		flag.has_param = message.params.size() > flag.param_index;
// 		flag.param = flag.has_param ? message.params[flag.param_index] : "";
// 		flag.acknowledge = false;
// 		flags.push_back(flag);
// 	}
// 	for (size_t i = 0; i < flags.size(); ++i)
// 	{
// 		if (flags[i].is_operator == true)
// 		{
// 			if (flags.size() > i + 1)
// 			continue;
// 		}
// 		if (flags[i].is_flag == false)
// 		{
// 			std::vector<std::string> params;
// 			params.push_back(std::string(1, flags[i].c));
// 			params.push_back(channel->getName());
// 			replies.push_back(createReply(ERR_UNKNOWNMODE, ERR_UNKNOWNMODE_STR, params));
// 			continue;
// 		}
// 		if (flags[i].requires_param && flags[i].has_param == false)
// 		{
// 			std::vector<std::string> params;
// 			params.push_back(std::string(1, flags[i].c));
// 			params.push_back(channel->getName());
// 			replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, params));
// 			continue;
// 		}
// 		if (flags[i].c == 'i')
// 		{
// 			if (first_appearances.i == false)
// 				continue;
// 			first_appearances.i = false;
// 			channel->setMode(flags[i].c, flags[i].active_operator == '+');
// 			flags[i].acknowledge = true;
// 			if (operator_to_be_added == true)
// 				corrected_mode_param += flags[i].active_operator; // We'll see
// 			corrected_mode_param += "i";
// 		}
// 		else if (flags[i].c == 't')
// 		{
// 			if (first_appearances.t == false)
// 				continue;
// 			first_appearances.t = false;
// 			channel->setMode(flags[i].c, flags[i].active_operator == '+');
// 			flags[i].acknowledge = true;
// 		}
// 		else if (flags[i].c == 'k')
// 		{
// 			if (first_appearances.k == false)
// 				continue;
// 			first_appearances.k = false;
// 			if (flags[i].active_operator == '+')
// 			{
// 				if (channel->getMode('k') == true)
// 				{
// 					replies.push_back(createReply(ERR_KEYSET, ERR_KEYSET_STR, channel->getName()));
// 					continue;
// 				}
// 				if (isKeyValid(message.params[flags[i].param_index]) == false)
// 					continue;
// 				channel->setKey(message.params[flags[i].param_index]);
// 				flags[i].acknowledge = true;
// 			}
// 			else // '-'
// 			{
// 				if (message.params[flags[i].param_index] == channel->getKey())
// 				{
// 					channel->setKey("");
// 					channel->setMode(flags[i].c, false);
// 					flags[i].acknowledge = true;
// 				}
// 			}
// 		}
// 		else if (flags[i].c == 'o')
// 		{
// 			// The parameter must be the nickname of a user in the channel
// 			if (channel->isUserInChannel(message.params[flags[i].param_index]) == false)
// 			{
// 				std::vector<std::string> params;
// 				params.push_back(message.params[flags[i].param_index]);
// 				params.push_back(channel->getName());
// 				replies.push_back(createReply(ERR_USERNOTINCHANNEL, ERR_USERNOTINCHANNEL_STR, params));
// 				continue;
// 			}
// 			if (flags[i].active_operator == '+')
// 				channel->promoteUser(message.params[flags[i].param_index]);
// 			else // '-'
// 				channel->demoteUser(message.params[flags[i].param_index]);
// 			flags[i].acknowledge = true;
// 		}
// 		else if (flags[i].c == 'l')
// 		{
// 			// Operator should be '+' at this point
// 			if (first_appearances.l == false)
// 				continue;
// 			first_appearances.l = false;
// 			long limit = limitStrToLong(message.params[flags[i].param_index]);
// 			if (limit == -1)
// 				continue;
// 			channel->setMode(flags[i].c, true); // I think this should work
// 			channel->setUserLimit(limit);
// 			flags[i].acknowledge = true;
// 		}
// 	}
// 	// TODO:
// 	// - Prepare the corrected mode string for the acknowledgment message
// 	// - Prepare the acknowledgment message
// 	// - Prepare the broadcast message for all users in the same channel as the sender

// 	t_message acknowledgement;
// 	acknowledgement.prefix = ":" + client->getUserPrefix();
// 	acknowledgement.command = "MODE";
// 	acknowledgement.params.push_back(channel->getName());
// 	acknowledgement.params.push_back(mode_param); // TODO
// 	acknowledgement.target_client_fds.insert(client->getSocket());

// 	t_message broadcast_message;
// 	broadcast_message.prefix = ":" + client->getUserPrefix();
// 	broadcast_message.command = "MODE";
// 	broadcast_message.params.push_back(channel->getName());
// 	// Continue

// 	std::vector<int> channel_users_fds = channel->getFds("users");
// 	for (std::vector<int>::iterator it = channel_users_fds.begin(); it != channel_users_fds.end(); ++it)
// 	{
// 		// This was done by copilot, I don't delete it yet so I can check it out tomorrow in case there is something useful

// 		// t_message ack_message;
// 		// ack_message.prefix = ":" + client->getUserPrefix();
// 		// ack_message.command = "MODE";
// 		// ack_message.params.push_back(channel->getName());
// 		// std::string ack_param;
// 		// for (size_t i = 0; i < flags.size(); ++i)
// 		// {
// 		// 	if (flags[i].acknowledge == false)
// 		// 		continue;
// 		// 	ack_param += " ";
// 		// 	ack_param += flags[i].active_operator;
// 		// 	ack_param += flags[i].c;
// 		// 	if (flags[i].requires_param)
// 		// 	{
// 		// 		ack_param += " ";
// 		// 		ack_param += message.params[flags[i].param_index];
// 		// 	}
// 		// }
// 		// ack_message.params.push_back(ack_param);
// 		// ack_message.target_client_fds.insert(*it);
// 		// replies.push_back(ack_message);
// 	}

// 	return replies;
// }


// // Updates, if needed, the active operator and returns true if it was updated
// static bool	updateActiveOperator( std::string & str, size_t i, char & active_operator )
// {
// 	if (i == 0)
// 	{
// 		if (str[i] == '-')
// 			active_operator = str[i];
// 		else
// 			active_operator = '+';
// 		return true;
// 	}
// 	else if (str[i] == '+' || str[i] == '-')
// 	{
// 		active_operator = str[i];
// 		return true;
// 	}
// 	return false;
// }

// static void getNextFlag( std::string & str, size_t i, char active_operator, Channel * channel )
// {
// 	char flag = str[i];
// 	if (std::string(CHANNEL_MODES).find(flag) == std::string::npos) // If the flag is not a valid channel mode
// 		return modeErrUnknownMode(std::string(1, flag), channel->getName());
// 	// If the flag IS a valid channel mode:
// 	/*
// 	We have to check a few things here:
// 	- Is this flag already been used in the current mode string?
// 		We check this by comparing with the ack_params string which will contain the correct mode string
// 	If it has not been already set:
// 	- Is this flag a mode that requires a parameter?
// 		If it is, we have to associate the flag with the next parameter in the message
// 	*/
// }

// std::vector<t_message> modeErrUnknownMode( const std::string & flag, std::string channel_name )
// {
// 	std::vector<t_message> replies;
// 	std::vector<std::string> params;
// 	params.push_back(flag);
// 	params.push_back(channel_name);
// 	replies.push_back(createReply(ERR_UNKNOWNMODE, ERR_UNKNOWNMODE_STR, params));
// 	return replies;
// }
