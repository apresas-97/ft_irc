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
	std::vector<t_message>	replies;
	Client * client = this->_current_client;

	std::string previous_modes = client->getModeString();

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
	for (size_t i = 0; i < parameter.size(); ++i)
	{
		char flag = flags[i].c;
		user_mode = client->hasMode(flag);
		set_mode = (flags[i].active_operator == '+');
		if (flags[i].is_operator)
			continue;
		if (std::string(USER_MODES).find(flag) != std::string::npos)
		{
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
		std::vector<std::string> params;
		params.push_back(channel->getName());
		params.push_back(channel->getModesString());
		if (channel->isUserInChannel(client->getNickname()) == false)
			params.push_back("");
		else
			params.push_back(channel->getModesParameters());

		replies.push_back(createReply(RPL_CHANNELMODEIS, RPL_CHANNELMODEIS_STR, params));
		return replies;
	}

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
	size_t param_index = 2;
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
		flag.requires_param = (flag.c == 'k' || flag.c == 'o' || (flag.c == 'l' && flag.active_operator == '+'));
		flag.param_index = flag.requires_param ? param_index++ : 0;
		flag.has_param = message.params.size() > flag.param_index;
		flag.param = flag.has_param ? message.params[flag.param_index] : "";
		flags.push_back(flag);
	}
	for (size_t i = 0; i < parameter.size(); ++i)
	{
		if (flags[i].is_operator)
		{
			continue;
		}
		if (flags[i].requires_param && flags[i].has_param == false)
		{
			if (flags[i].c == 'l')
				replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, message.command));
			continue;
		}
		switch (flags[i].c)
		{
			case 'i':
			{
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
				if (first_appearances.k == false) continue;
				first_appearances.k = false;
				if (flags[i].active_operator == '+')
				{
					if (channel->getMode('k') == true)
					{
						replies.push_back(createReply(ERR_KEYSET, ERR_KEYSET_STR, channel->getName()));
						continue;
					}
					std::string key = flags[i].param;
					if (key.size() > 23)
						key.resize(23);
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
				if (first_appearances.l == false) continue;
				first_appearances.l = false;
				long limit = -1;
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
				std::vector<std::string> params;
				params.push_back(std::string(1, flags[i].c));
				params.push_back(channel->getName());
				replies.push_back(createReply(ERR_UNKNOWNMODE, ERR_UNKNOWNMODE_STR, params));
			}
		}
	}


	if (!corrected_mode_param.empty() && used_params.empty())
		corrected_mode_param += " ";
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

	return replies;
}
