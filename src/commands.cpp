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