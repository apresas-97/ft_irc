#include "Server.hpp"

/* apresas-:
	Some info about commoands:

	Commands will only care about the parameters they expect
	If a command expects 1 paramter, it will just process the first parameter,
	the rest will be ignored.
*/

/*
Command: PASS
Parameters: <password>

The PASS command is used to set a 'connection password'.
The optional password can and MUST be set before any attempt to register
the connection is made. This requires that user send a PASS command before
sending the NICK/USER combination.
*/
std::vector<t_message> Server::cmdPass( t_message & message ) {
	Client & client = *this->_current_client;
	std::vector<t_message> replies;

	if (message.params.size() < 1) {
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client.getNickname()));
		return replies;
	}
	if (message.params[0] == this->_password)
		client.setAuthorised(true);
	else
		replies.push_back(createReply(ERR_PASSWDMISMATCH, ERR_PASSWDMISMATCH_STR));
	return replies;
}

#include <algorithm> // for std::find
/*
Command: NICK
Parameters: <nickname>
NICK command is used to give user a nickname or change the existing one.
*/
std::vector<t_message> Server::cmdNick( t_message & message ) {
	Client & client = *this->_current_client;
	std::vector<t_message> replies;

	if (client.isAuthorised() == false || client.isRegistered() == false) {
		replies.push_back(createReply(ERR_RESTRICTED, ERR_RESTRICTED_STR));
		return replies;
	}
	if (client.getMode('r') == true) {
		replies.push_back(createReply(ERR_RESTRICTED, ERR_RESTRICTED_STR));
		return replies;
	}
	if (message.params.size() < 1) {
		replies.push_back(createReply(ERR_NONICKNAMEGIVEN, ERR_NONICKNAMEGIVEN_STR));
		return replies;
	}
	std::string nickname = message.params[0];
	if (irc_isValidNickname(nickname) == false) {
		replies.push_back(createReply(ERR_ERRONEUSNICKNAME, ERR_ERRONEUSNICKNAME_STR, nickname));
		return replies;
	}

	// apresas-: Check for nickname collision ? Idk how to do this yet
	// This triggers ERR_NICKCOLLISION

	// apresas-: Check if the nickname cannot be chosen because of the nick delay mechanism
	// I don't even know if we'll implement that yet
	// If the name is not available, return ERR_UNAVAILRESOURCE

	std::vector<std::string>::iterator it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), nickname);
	if (it != this->_taken_nicknames.end()) {
		replies.push_back(createReply(ERR_NICKNAMEINUSE, ERR_NICKNAMEINUSE_STR, nickname));
		return replies;
	}
	// If all went well, change the user's nickname and return its own message as acknowledgement
	client.setNickname(nickname);
	this->_taken_nicknames.push_back(nickname);
	// It should now be removed from the taken nicknames list, but not immediately
	// I need to look this up and see how it truly works
	// I think it's removed after a certain amount of time, but I'm not sure

	/*
	apresas-: TO-DO: Send a message to all channels the user is in that the nickname has changed
	The message should be something like:
	:oldnickname NICK newnickname
	It should be broadcasted to all users in the same channels as the user
	And probably only to those that would be allowed to see the user???? Idk
	I need to look this up
	Ughhhhh
	*/
}

/*
Command: USER
Parameters: <username> <mode> <unused> <realname>
USER command is used at the beginning of connection to specify the identity of the user.
<mode> should be numeric, and it's a bitmask for modes 'w'(bit 2) and 'i'(bit 3). The rest of bits
are irrelevant.
<realname> may contain space characters.

New info:
Apparently, nowadays, the <mode> parameter is ignored completely by most servers.
*/
std::vector<t_message>	Server::cmdUser( t_message & message ) {
	std::vector<t_message> replies;
	if (message.params.size() < 4) {
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, this->_current_client->getNickname()));
		return replies;
	}
	if (this->_current_client->getNickname().empty() == true) {
		// Idk what to do here, the nickname should have been set already
		// If not, the registration can't continue
		// I need to figure this out
	}
	Client & client = *this->_current_client;
	if (client.isRegistered() == true) {
		replies.push_back(createReply(ERR_ALREADYREGISTRED, ERR_ALREADYREGISTRED_STR));
		return replies;
	}
	// apresas-: TO-DO: Check that the username and realname are not too long
	// But if they are, I don't know how to handle it yet because the protocol doesn't
	// specify what to do in that case
	// It doesn't really even specify a limit for username or realname length
	// But we should set some reasonable limit or we could be vulnerable to
	// all sort of buffer overflow attacks, etc.
	client.setUsername(message.params[0]);
	client.setRealname(message.params[3]);

	client.setRegistered(true);

	// TO-DO:
	// Prepare the welcome message reply
	replies.push_back(createReply(RPL_WELCOME, RPL_WELCOME_STR, client.getUserIdentifier()));
	std::vector<std::string> yourhost_params;
	yourhost_params.push_back(this->getName());
	yourhost_params.push_back(this->getVersion());
	replies.push_back(createReply(RPL_YOURHOST, RPL_YOURHOST_STR, yourhost_params));
	replies.push_back(createReply(RPL_CREATED, RPL_CREATED_STR, this->getStartTimeStr())); // TO-DO: Get the creation date from somewhere
	std::vector<std::string> myinfo_params;
	myinfo_params.push_back(this->getName());
	myinfo_params.push_back(this->getVersion());
	myinfo_params.push_back(USER_MODES); // Here will go all of the available user modes
	myinfo_params.push_back(CHANNEL_MODES); // Here will go all of the available channel modes
	replies.push_back(createReply(RPL_MYINFO, RPL_MYINFO_STR, myinfo_params));

	return replies;
}

/*
Command: MODE

This doesn't work yet, but it's pretty close I think
*/
std::vector<t_message> Server::cmdMode( t_message & message ) {
	std::vector<t_message>	replies;
	Client & client = *this->_current_client;
	if (message.params.size() < 1) {
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, client.getNickname()));
		return replies;
	}

	if (client.getNickname() != message.params[0]) {
		if (isUserInServer(message.params[0]) == true) // TO-DO
			replies.push_back(createReply(ERR_USERSDONTMATCH, ERR_USERSDONTMATCH_STR));
		else if (isChannelInServer(message.params[0] == true))// TO-DO
			return cmdChanMode(message);
		else
			replies.push_back(createReply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, message.params[0]));
		return replies;
	}

	if (message.params.size() == 1) {
		replies.push_back(createReply(RPL_UMODEIS, RPL_UMODEIS_STR, get_mode_string(modes))); // TO-DO
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
	for (size_t i = 0; i < param.size(); ++i) {
		if (param[i] == '+') {
			if (operation == false || first_operator == true)
				insert_operator = true;
			operation = true;
			first_operator = false;
		} else if (param[i] == '-') {
			if (operation == true)
				insert_operator = true;
			operation = false;
			first_operator = false;
		} else {
			char mode = param[i];
			if (std::string(USER_MODES).find(mode) == std::string::npos) {
				if (unknown_flag_set == false) {
					replies.push_back(createReply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR, {}));
					unknown_flag_set = true;
				}
				continue;
			}
			bool has_mode = client.hasMode(mode, /*channel_name?*/); // TO-DO
			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) {
				if (operation == true && (mode == 'o' || mode == 'O'))
					continue;
				if (operation == false && mode == 'r')
					continue;
				if (mode == 'a')
					continue;
				client.setMode(mode, operation, /*channel_name?*/); // TO-DO
				has_effect = true;
				if (insert_operator == true) {
					valid_modes.push_back((operation ? '+' : '-') );
					insert_operator = false;
				}
				valid_modes.push_back(mode);
			}
		}
	}
	bool implicit_plus = false;
	if (!valid_modes.empty()) {
		for (size_t i = 0; i < valid_modes.size(); ++i) {
			if (i == 0 && valid_modes[i] != '+' && valid_modes[i] != '-') {
				correct_param += '+';
				implicit_plus = true;
			}
			if (valid_modes[i] == '+' && implicit_plus == true) {
				implicit_plus = false;
				continue;
			}
			correct_param += valid_modes[i];
		}
	}
	if (has_effect)
		replies.push_back(createReply(message, correct_param, client.getNickname()));
	return replies;
}
