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
           RPL_WHOREPLY                    RPL_ENDOFWHO

   Examples:

   WHO *.fi                        ; List all users who match against
                                   "*.fi".

   WHO jto* o                      ; List all users with a match against
                                   "jto*" if they are an operator.
*/

#include "Server.hpp"

std::vector<t_message> Server::cmdWho( t_message & message )
{
	//	....	
}

