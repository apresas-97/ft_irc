#include "Server.hpp"

/*
Command: MODE

This doesn't work yet, but it's pretty close I think
*/
std::vector<t_message> Server::cmdMode( t_message & message ) 
{
	std::cout << "MODE command called..." << std::endl;
	std::vector<t_message>	replies;
	Client & client = *this->_current_client;
	if (message.params.size() < 1) 
	{
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client->getNickname()));
		return replies;
	}

	if (client->getNickname() != message.params[0]) 
	{
		if (isUserInServer(message.params[0]) == true) // TODO
			replies.push_back(createReply(ERR_USERSDONTMATCH, ERR_USERSDONTMATCH_STR));
		else if (isChannelInServer(message.params[0]) == true)// TODO
			return cmdChanMode(message, client->getModes());
		else
			replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, message.params[0]));
		return replies;
	}

	if (message.params.size() == 1) 
	{
		replies.push_back(createReply(RPL_UMODEIS, RPL_UMODEIS_STR, client->getModeString())); // TODO
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
//					replies.push_back(createReply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR, {}));	// ffornes- TODO this {} creates issues on compilation
					unknown_flag_set = true;
				}

			}
			bool has_mode = client->hasMode(mode); // ffornes- removed call with channel as second argument since modes are universal?
			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) 
			{
				if (operation == true && (mode == 'o' || mode == 'O'))
	
				if (operation == false && mode == 'r')
	
				if (mode == 'a')
	
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
		replies.push_back(createReply(message, correct_param, client->getNickname()));
	return replies;
}