#include "Server.hpp"

/*
Command: VERSION
Parameters: [ <target> ]

The VERSION command is used to query the version of the server program.

The optional parameter <target> is used to query the version of the server program
which the client is not directly connected to.

Since our IRC server does not have server-to-server communication,
the <target> param will only be matched against the server name.

Numeric Replies:
- ERR_NOSUCHSERVER
- RPL_VERSION
*/
std::vector<t_message>	Server::cmdVersion( t_message & message )
{
	std::vector<t_message> replies;
	std::vector<std::string> reply_params;
	// TODO - Decide what to do about the comments, this is not what other famous servers do
	std::string comments;
	comments = USER_MODES;
	comments += " ";
	comments += CHANNEL_MODES;

	if (message.params.size() > 0 && message.params[0] != this->getName())
		replies.push_back(createReply(ERR_NOSUCHSERVER, ERR_NOSUCHSERVER_STR, message.params[0]));
	else
	{
		reply_params.push_back(this->getVersion());
		reply_params.push_back("");
		reply_params.push_back(this->getName());
		reply_params.push_back(comments);
		replies.push_back(createReply(RPL_VERSION, RPL_VERSION_STR, reply_params));
	}
	return replies;
}
