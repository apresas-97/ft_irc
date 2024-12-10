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
int	Server::cmdPass( t_message & message ) {
	if (message.params.size() < 1)
		return ERR_NEEDMOREPARAMS;
	if (message.params[0] == this->_password)
		this->_clients[message.sender_client_fd].setAuthorised(true);
	return ERR_PASSWDMISMATCH;
}

#include <algorithm> // for std::find
/*
Command: NICK
Parameters: <nickname>
NICK command is used to give user a nickname or change the existing one.
*/
int	Server::cmdNick( t_message & message ) {
	Client & client = this->_clients[message.sender_client_fd];

	if (client.isAuthorised() == false)
		return ERR_RESTRICTED;
	if (client.isRegistered() == false)
		return ERR_RESTRICTED;
	if (client.getMode('r') == true)
		return ERR_RESTRICTED;

	if (message.params.size() < 1) {
		return ERR_NONICKNAMEGIVEN;
	}

	std::string nickname = message.params[0];
	if (irc_isValidNickname(nickname) == false) {
		return ERR_ERRONEUSNICKNAME;
	}

	// apresas-: Check for nickname collision ? Idk how to do this yet
	// This triggers ERR_NICKCOLLISION

	// apresas-: Check if the nickname cannot be chosen because of the nick delay mechanism
	// I don't even know if we'll implement that yet
	// If the name is not available, return ERR_UNAVAILRESOURCE

	std::vector<std::string>::iterator it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), nickname);
	if (it != this->_taken_nicknames.end())
		return ERR_NICKNAMEINUSE;
	return 0;
}

#include <sstream>
/*
Command: USER
Parameters: <username> <mode> <unused> <realname>
USER command is used at the beginning of connection to specify the identity of the user.
<mode> should be numeric, and it's a bitmask for modes 'w'(bit 2) and 'i'(bit 3). The rest of bits
are irrelevant.
<realname> may contain space characters.
*/
int	Server::cmdUser( t_message & message ) {
	if (message.params.size() < 4)
		return ERR_NEEDMOREPARAMS;
	Client & client = this->_clients[message.sender_client_fd];
	if (client.isRegistered() == true)
		return ERR_ALREADYREGISTRED;

	// apresas-: TO-DO: Check that the username and realname are not too long
	// But if they are, I don't know how to handle it yet because the protocol doesn't
	// specify what to do in that case
	// It doesn't really even specify a limit for username or realname length
	// But we should set some reasonable limit or we could be vulnerable to
	// all sort of buffer overflow attacks, etc.
	client.setUsername(message.params[0]);
	client.setRealname(message.params[3]);

	std::istringstream iss(message.params[1]);
	int mode_bitmask = 0;

	iss >> mode_bitmask;
	if (iss.fail())
		mode_bitmask = 0; // default to 0 since it's not specified in the IRC protocol
	if (mode_bitmask & (1 << 2))
		client.setMode('i', true);
	else
		client.setMode('i', false);
	if (mode_bitmask & (1 << 3))
		client.setMode('w', true);
	else
		client.setMode('w', false);

	client.setRegistered(true);
	return 0;
}
