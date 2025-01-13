#include "Server.hpp"

/*
Command: MODE

This doesn't work yet, but it's pretty close I think
*/
std::vector<t_message> Server::cmdMode( t_message & message ) 
{
	std::cout << "MODE command called..." << std::endl;
	std::vector<t_message>	replies;
	t_message				reply;
	Client * client = this->_current_client;

	// Param count check
	if (message.params.size() < 1) 
	{
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname());
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}

	// // Maybe
	// if (irc_isChannelPrefix(message.params[0][0])) // If param starts with channel prefix, then it must be a channel
	// {
	// 	// Channel MODE message:
	// 	return cmdModeChannel(message, client->getModes());
	// }
	// else // If not, then it must be a nickname
	// {
	// 	// User MODE message:
	// 	return (cmdModeUser(message));
	// }

	// If the given parameter is a NOT a nickname of a client in this server
	if (isUserInServer(message.params[0]) == false)
	{
		replies = cmdModeChannel(message);
	}

	return cmdModeUser(message);
	// FIN
	///////////

		if (isUserInServer(message.params[0]) == true) // TODO
		{
			reply = createReply(ERR_USERSDONTMATCH, ERR_USERSDONTMATCH_STR);
			reply.target_client_fd = message.sender_client_fd;
			reply.sender_client_fd = _serverFd;
			replies.push_back(reply);
		}
		else if (isChannelInServer(message.params[0]) == true)// TODO
			return cmdChanMode(message, client->getModes());
		else
		{
			reply = createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, message.params[0]);
			reply.target_client_fd = message.sender_client_fd;
			reply.sender_client_fd = _serverFd;
			replies.push_back(reply);
		}
		return replies;
	}

	if (message.params.size() == 1) 
	{
		reply = createReply(RPL_UMODEIS, RPL_UMODEIS_STR, client->getModeString()); // TODO
		replies.push_back(reply);
		return replies;
	}

	// This is kinda ugly, but I think it works for now
	std::vector<char> valid_modes;
	std::string correct_param;
	bool has_effect = false;
	bool operation = true;
	bool insert_operator = false;
	bool first_operator = true;
	bool unknown_flag_set = false;
	std::string param = message.params[1];
	for (size_t i = 0; i < param.size(); ++i) 
	{
		if (param[i] == '+') 
		{
			if (operation == false || first_operator == true)
				insert_operator = true;
			operation = true;
			first_operator = false;
		}
		else if (param[i] == '-') 
		{
			if (operation == true)
				insert_operator = true;
			operation = false;
			first_operator = false;
		}
		else 
		{
			char mode = param[i];
			if (std::string(USER_MODES).find(mode) == std::string::npos) 
			{
				if (unknown_flag_set == false) 
				{
//					reply = createReply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR, {}); // incorrect call
					reply.target_client_fd = message.sender_client_fd;
					reply.sender_client_fd = _serverFd;
					replies.push_back(reply);
					unknown_flag_set = true;
				}

			}
			bool has_mode = client->hasMode(mode); // ffornes- removed call with channel as second argument since modes are universal?
			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) 
			{
				if (operation == true && (mode == 'o' || mode == 'O'))
					// ??
				if (operation == false && mode == 'r')
					// ??
				if (mode == 'a')
					// ??
				client->setMode(mode, operation); // ffornes- same as comment above
				has_effect = true;
				if (insert_operator == true) 
				{
					valid_modes.push_back((operation ? '+' : '-') );
					insert_operator = false;
				}
				valid_modes.push_back(mode);
			}
		}
	}
	bool implicit_plus = false;
	if (!valid_modes.empty()) 
	{
		for (size_t i = 0; i < valid_modes.size(); ++i) 
		{
			if (i == 0 && valid_modes[i] != '+' && valid_modes[i] != '-') 
			{
				correct_param += '+';
				implicit_plus = true;
			}
			if (valid_modes[i] == '+' && implicit_plus == true) 
			{
				implicit_plus = false;

			}
			correct_param += valid_modes[i];
		}
	}
	if (has_effect)
	{
		reply = createReply(message, correct_param, client->getNickname());
		// TODO set target_client_fd and sender_client_fd
		replies.push_back(reply);
	}
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

Alternative, additional parameters can be given to change the nick's modes.

The flag 'a' MUST NOT be toggled using this command. Instead, it is changed
with the AWAY command.

Attempts to set the +o or +O flag SHOULD be ignored.
There is no restriction, however, on anyone deopping themselves using -o or -O.

Attempts to set the -r flag SHOULD be ignored.
There is no restriction, however, on anyone deopping themselves using +r.
Though this flag is usually set

Numeric replies:
- ERR_NEEDMOREPARAMS : If no parameters are given (This was already handled in the main MODE command)
- ERR_USERSDONTMATCH : If the given parameter is not the nickname of the sender
- ERR_UMODEUNKNOWNFLAG : If an unknown flag is given (Doesn't negate the known flags in the parameters)
- RPL_UMODEIS : Reply with the current modes of the user (if no additional parameters are given)
*/
std::vector<t_message>	Server::cmdModeUser( t_message & message )
{
	// This function is only called if the given parameter is a nickname of a client in this server
	std::vector<t_message>	replies;
	Client * client = this->_current_client;

	// Check if the given nickname is NOT the same as the sender's nickname
	if (message.params[0] != client->getNickname())
	{
		// ERR_USERSDONTMATCH reply
		replies.push_back(createReply(ERR_USERSDONTMATCH, ERR_USERSDONTMATCH_STR));
		return replies;
	}

	if (message.params.size() == 1) 
	{
		// RPL_UMODEIS reply
		replies.push_back(createReply(RPL_UMODEIS, RPL_UMODEIS_STR, client->getModeString()));
		return replies;
	}

	std::vector<char>	valid_modes;
	std::string			correct_param;
	std::string			parameter = message.params[1];

	bool	unknown_flag_reply_set = false;

	char	active_operator = '\0';
	bool	insert_operator = false;
	bool	is_first_operator = true;

	for (size_t = 0; i < parameter.size(); ++i)
	{
		if (parameter[i] == '+')
		{
			if (active_operator != '+') // If active operator is '-' or it has not been set yet
				insert_operator = true;
			active_operator = '+';
		}
		else if (parameter[i] == '-')
		{
			if (active_operator == '+') // If active operator is '+'
				insert_operator = true;
			active_operator = '-';
		}
		else
		{
			char flag = parameter[i];
			if (std::string(USER_MODES).find(mode) == std::string::npos)
			{
				if (unknown_flag_reply_set == false)
				{
					replies.push_back(createReply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR));
					unknown_flag_reply_set = true;
				}
				continue;
			}
			bool has_mode = client->hasMode(flag);
			if ((active_operator == '+' && has_mode == false) || (active_operator == '-' && has_mode == true))
			{
				if (active_operator == '+' && (mode == 'o' || mode == 'O'))
					continue;
				if (active_operator == '-' && (mode == 'r'))
					continue;
				if (mode == 'a')
					continue;
				client->setMode(flag, active_operator == '+');
				if (insert_operator == true)
				{
					valid_modes.push_back(active_operator);
					insert_operator = false;
				}
				valid_modes.push_back(flag);
			}
		}
	}
	bool implicit_plus = false;
	if (!valid_modes.empty())
	{
		for (size_t i = 0; i < valid_modes.size(); ++i)
		{
			if (i == 0 && valid_modes[i] != '+' && valid_modes[i] != '-')
			{
				correct_param += '+';
				implicit_plus = true;
			}
			if (valid_modes[i] == '+' && implicit_plus == true)
			{
				implicit_plus = false;
				continue;
			}
			correct_param += valid_modes[i];
		}
	}
	if (has_effect)
	{
		// Create the acknowledgement message with the corrected parameter
		replies.push_back(createReply(message, correct_param, client->getNickname()));
	}
	return replies;
}


/*
(Channel mode message)
Command: MODE
Parameters: <channel> *( ( "-" / "+" ) *<modes> *<modeparams> )

The channel MODE message is used to query and change the characteristics
of a channel.

// Later I've got to add the explanation for each mode and its optional parameters
// Here they are: https://datatracker.ietf.org/doc/html/rfc2811#section-4

- O : give "channel creator" status

- o : give/take channel operator privilege (WILL implement)

- v : give/take the voice privilege
	I think this is to add a user to the channel's voice list, which allows them to
	speak in a moderated channel without being an operator
	This adds the prefix '+' to the user's nickname in the channel
	(Probably not going to implement this)

- a : toggle the anonymous channel flag.
	When set, all conversations and people joining/parting will be viewed as
	"anonymous!anonymous@anonymous".
	Users quitting are seen as users parting the channel with no reason.
	(Might? Implement this idk)

- i : toggle invite-only.
	When toggled, only channel operators can invite users to the channel.
	(I think)
	(WILL implement)

- m : toggle moderated channel
	When toggled, only users with voice or operator status can speak in the channel
	(Probably will implement this)

- n : toggle no messages to channel from clients on the outside
	When toggled, disallows messages from users who are not channel members entering
	the channel (I don't understand what this means exactly)

- q : toggle quiet channel
	This is annoying to implement with our current structure, so I'm not going to do it

- s : toggle the secret channel flag
	This is annoying to implement with our current structure, so I'm not going to do it

- r : reop thing, this looks overly complicated, so I'm not going to implement it

- t : toggle the topic settable by channel operator only flag
	When toggled, only channel operators can change the topic
	(WILL implement)

- k : set/remove the channel key
	Use +k <key> to set the key of the channel
	Use -k <current_key> to remove the key
	(WILL implement)

- l : set the user limit to channel
	Use +l <limit> to set the limit of users in the channel. i.e. +l 10
	Use -l to remove the limit
	If the number of members is greater than the set limit, it's okay for them to stay
	(WILL implement)

- b : set/remove ban mask to keep users out
	This works with masks, and that's annoying, so I'm not going to implement it

- e : set/remove an exception mask to override a ban mask
	This works with masks, and that's annoying, so I'm not going to implement it

- I : set/remove an invitation mask to automatically override the invite-only flag
	This works with masks, and that's annoying, so I'm not going to implement it

There is a limit of 3 change per command for nodes that take a parameter

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

// Unsure about which of these we'll actually need to implement
*/
std::vector<t_message>	Server::cmdModeChannel( t_message & message )
{
	std::vector<t_message>	replies;
	Client * client = this->_current_client;
	Channel * channel = findChannel(message.params[0]);

	// If the given parameter is not a channel in this server
	if (channel == NULL)
	{
		replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, message.params[0]));
		return replies;
	}

	// If the sender of this message is not in the channel
	if (channel->isUserInChannel(client->getNickname()) == false)
	{
		std::vector<std::string> params;
		params.push_back(client->getNickname());
		params.push_back(channel->getName());
		replies.push_back(createReply(ERR_USERNOTINCHANNEL, ERR_USERNOTINCHANNEL_STR, params));
		return replies;
	}

	std::string	valid_modes;
	std::vector<std::string> correct_params;
	std::string parameter = message.params[1];


	size_t	i = 2;
	char	active_operator = '\0';
	bool	insert_operator = false;
	bool	is_first_operator = true;

	for (size_t i = 0; i < parameter.size(); ++i)
	{
		if (parameter[i] == '+')
		{
			if (active_operator != '+')
				insert_operator = true;
			active_operator = '+';
		}
		if (parameter[i] == '-')
		{
			if (active_operator == '+')
				insert_operator = true;
			active_operator = '-';
		}
		else
		{
			char mode = parameter[i];
			if (std::string(CHANNEL_MODES).find(mode) == std::string::npos)
			{
				std::vector<std::string> params;
				params.push_back(std::string(mode));
				params.push_back(channel->getName());
				replies.push_back(createReply(ERR_UNKNOWNMODE, ERR_UNKNOWNMODE_STR, params));
				continue;
			}
			bool has_mode = channel->hasMode(mode);
			bool set_flag = false;
			// Ugly
			if (mode == 'k' && active_operator == '+' && has_mode == true)
			{
				replies.push_back(createReply(ERR_KEYSET, ERR_KEYSET_STR, channel->getName()));
				continue;
			}
			if ((active_operator == '+' && has_mode == false) || (active_operator == '-' && has_mode == true))
			{
				set_flag = true;
				std::string param = message.command + " " + active_operator + mode;
				if (insert_operator == true)
				{
					valid_modes.push_back(active_operator);
					insert_operator = false;
				}
				if (active_operator == '+')
				{
					if (mode == 'k')
					{
						if (message.params.size() <= i)
							replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, param));
						else
						{
							channel->setKey(message.params[i]);
							channel->setMode(mode, active_operator == '+');
						}
					}
					else if (mode == 'l')
					{
						if (message.params.size() <= i)
							replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, param));
						else
						{
							// TODO : convert string to size_t before sending it
							// channel->setUserLimit(message.params[i]);
							channel->setMode(mode, active_operator == '+');
						}
					}
					else if (mode == 'o')
					{
						if (message.params.size() <= i)
							replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, param));
						else
						{
							// If user is not in channel
							// ERR_USERNOTINCHANNEL
							// else
							channel->promoteUser(message.params[i]);
							channel->setMode(mode, active_operator == '+');
						}
					}
					else
					{
						channel->setMode(mode, active_operator == '+');
					}
				}
				else if (active_operator == '-')
				{
					if (mode == 'k')
					{
						if (message.params.size() <= i)
							replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, param));
						else if (message.params[i] == channel->getKey())
						{
							channel->setKey("");
							channel->setMode(mode, active_operator == '+');
						}
					}
					else if (mode == 'l')
					{
						channel->setUserLimit(0);
						channel->setMode(mode, active_operator == '+');
					}
					else if (mode == 'o')
					{
						if (message.params.size() <= i)
							replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, param));
						else
						{
							// If user is not in channel
							// ERR_USERNOTINCHANNEL
							// else
							channel->demoteUser(message.params[i]);
							channel->setMode(mode, active_operator == '+');
						}
					}
					else
					{
						channel->setMode(mode, active_operator == '+');
					}
				}
			}
		}
	}
}

/* channel MODE behavior:

I will treat it like this:

If a +/- is found, that implies a change in a mode

Read the given modes after the operator

If a mode is found, it will be matched against the next parameter if it takes one

If it doesn't take a parameter, it will be set/unset

If there is a fourth set/unset operation with parameter, it will be ignored ?
*/