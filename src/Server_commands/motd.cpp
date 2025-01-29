#include "Server.hpp"
#include <fstream>

/*
Command: MOTD
Parameters: [ <target> ]

The MOTD command is used to get the "Message Of The Day" of the given server,
or current server if <target> is omitted.

Numeric replies:
- RPL_MOTDSTART : Start of MOTD
- RPL_MOTD : Body of MOTD
- RPL_ENDOFMOTD : End of MOTD
- ERR_NOMOTD : No MOTD file found
*/
std::vector<t_message> Server::cmdMotd( t_message & message )
{
	std::vector<t_message> replies;
	Client * client = this->_current_client;
	std::ifstream file;

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}

	if (message.params.size() > 0 && message.params[0] != this->getName())
	{
		replies.push_back(createReply(ERR_NOSUCHSERVER, ERR_NOSUCHSERVER_STR, message.params[0]));
		return replies;
	}

	file.open("motd.conf");
	if (!file.is_open() || !file.good() || file.fail())
	{
		replies.push_back(createReply(ERR_NOMOTD, ERR_NOMOTD_STR));
		return replies;
	}

	std::string line;
	replies.push_back(createReply(RPL_MOTDSTART, RPL_MOTDSTART_STR, this->getName()));
	while (getline(file, line))
		replies.push_back(createReply(RPL_MOTD, RPL_MOTD_STR, line));
	replies.push_back(createReply(RPL_ENDOFMOTD, RPL_ENDOFMOTD_STR));

	if (replies.size() == 2)
	{
		replies.clear();
		replies.push_back(createReply(ERR_NOMOTD, ERR_NOMOTD_STR));
	}

	file.close();

	return replies;
}
