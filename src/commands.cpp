#include "Server.hpp"

/* apresas-:
	Some info about commoands:

	Commands will only care about the parameters they expect
	If a command expects 1 paramter, it will just process the first parameter,
	the rest will be ignored.
*/

std::vector<t_message> Server::cmdChanMode( t_message & message, t_mode modes ) 
{
	std::cout << "CHANMODE command called..." << std::endl;
	std::vector<t_message> replies;
/*
	std::cout << "Channel mode part, not implemented yet" << std::endl;
	// Some modes take extra parameters, like +k <key> or +l <limit>
	// Note that there is a maximum limit of 3 changes per command for modes that require parameters
	// Unsure what this is saying exactly

	if (message.params.size() == 1) 
	{
		if (message.params[0][0] == '+') 
		{
			replies.push_back(reply(ERR_NOCHANMODES, ERR_NOCHANMODES_STR, { message.params[0] }));
			return replies;
		}
		replies.push_back(reply(RPL_CHANNELMODEIS, RPL_CHANNELMODEIS_STR, { message.params[0], get_mode_string(modes), //get_mode_params_string(modes) })); // TODO 
		return replies;
	}
*/
	//////////////////////////////////////////////// BEGGINING OF COMMENT //////////////////////////////////////////////////////////////
	/*
	Replies:
		ERR_KEYSET
			If the channel already has a key and the user tries to set it again
		ERR_CHANOPRIVSNEEDED
			If the user tries to set a mode that requires channel operator privileges
		ERR_USERNOTINCHANNEL
			If the mode to be set specifies as parameter a user, and that user is not in the channel
			i.e. +o <nickname>, trying to set as operator someone who doesn't belong to the channel
		ERR_UNKNOWNMODE
			If the mode is either invalid or not supported by the channel in some way
			i.e. +k <key>, on a channel that doesn't support keys
			or +X, since it's not a valid mode period
	*/
///////////////////////////////////////// ENDOF COMMENT //////////////////////////////////////////////////////////////////////////////////////
	// Provisional: 
	/*
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
		} else if (param[i] == '-') 
		{
			if (operation == true)
				insert_operator = true;
			operation = false;
			first_operator = false;
		} else 
		{
			char mode = param[i];
			if (std::string(USER_MODES).find(mode) == std::string::npos) 
			{
				if (unknown_flag_set == false) 
				{
					replies.push_back(reply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR, {}));
					unknown_flag_set = true;
				}

			}
			bool has_mode = modes.getMode(mode);
			std::cout << "has mode " << mode << " ? " << has_mode << std::endl;
			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) 
			{
				if (operation == true && (mode == 'o' || mode == 'O'))
	
				if (operation == false && mode == 'r')
	
				if (mode == 'a')
	
				modes.setMode(mode, operation);
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
		replies.push_back(reply(message, correct_param, client_nickname));
	*/
	return replies;
	(void)message;
	(void)modes;
}
