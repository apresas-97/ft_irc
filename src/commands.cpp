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
void	Server::cmdPass( t_message & message ) {
	if (message.params.size() < 1) {
		// Send ERR_NEEDMOREPARAMS
		return;
	}

	if (message.params[0] == this->_password) {
		// Authorize the client who sent this message
	} else {
		// Send ERR_PASSWDMISMATCH
		return;
	}
}

/*
Command: NICK
Parameters: <nickname>
NICK command is used to give user a nickname or change the existing one.
*/
void	Server::cmdNick( t_message & message ) {
	if (message.params.size() < 1) {
		// Send ERR_NONICKNAMEGIVEN
		return;
	}
	/*
	Must check:
		if the nickname is already in use
			ERR_NICKNAMEINUSE
		if the nickname is valid
			ERR_ERRONEUSNICKNAME
		if there is a nickname collision (maybe)
			ERR_NICKCOLLISION
		if ?
			ERR_RESTRICTED
		if ?
			ERR_UNAVAILABLERESOURCE

	*/
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
void	Server::cmdUser( t_message & message ) {
	if (message.params.size() < 4) {
		// Send ERR_NEEDMOREPARAMS
		return;
	}
	std::string username = message.params[0];
	std::string realname = message.params[3];

	std::istringstream iss(message.params[1]);
	int mode_bitmask = 0;

	iss >> mode_bitmask;
	if (iss.fail())
		mode_bitmask = 0; // default to 0 since it's not specified in the IRC protocol
	if (mode_bitmask & (1 << 2))
		// Set user's +i
		// client->setMode(true, 'i');
		// client->setMode("+i");
	else
		// Set user's -i
		// client->setMode(false, 'i');
		// client->setMode("-i");
	if (mode_bitmask & (1 << 3))
		// Set user's +w
	else
		// Set user's -w
}