#include "Server.hpp"
#include <ctime>
#include <iomanip>

static std::string cmdTimeTimestamp( void )
{
	time_t time = std::time(0);
	std::tm *now = std::localtime(&time);

	std::ostringstream oss;
	switch (now->tm_wday) 
	{
		case 0:
			oss << "Sunday ";
			break;
		case 1:
			oss << "Monday ";
			break;
		case 2:
			oss << "Tuesday ";
			break;
		case 3:
			oss << "Wednesday ";
			break;
		case 4:
			oss << "Thursday ";
			break;
		case 5:
			oss << "Friday ";
			break;
		case 6:
			oss << "Saturday ";
			break;
	}
	switch (now->tm_mon) 
	{
		case 0:
			oss << "January ";
			break;
		case 1:
			oss << "February ";
			break;
		case 2:
			oss << "March ";
			break;
		case 3:
			oss << "April ";
			break;
		case 4:
			oss << "May ";
			break;
		case 5:
			oss << "June ";
			break;
		case 6:
			oss << "July ";
			break;
		case 7:
			oss << "August ";
			break;
		case 8:
			oss << "September ";
			break;
		case 9:
			oss << "October ";
			break;
		case 10:
			oss << "November ";
			break;
		case 11:
			oss << "December ";
			break;
	}
	oss << std::setw(2) << std::setfill('0') << now->tm_mday << " ";
	oss << std::setw(4) << std::setfill('0') << now->tm_year + 1900 << " -- ";
	oss << std::setw(2) << std::setfill('0') << now->tm_hour << ":";
	oss << std::setw(2) << std::setfill('0') << now->tm_min << ":";
	oss << std::setw(2) << std::setfill('0') << now->tm_sec << " +01:00";

	return oss.str();
}

/*
Command: TIME
Parameters: [ <server> ]

The TIME command is used to query the local time of the server.
The optional parameter <target> is used to query the local time
of the server program which the client is not directly connected to.

Since our IRC server does not have server-to-server communication,
the <target> param will only be matched against the server name.

Numeric Replies:
- ERR_NOSUCHSERVER
- RPL_TIME
*/
std::vector<t_message>	Server::cmdTime( t_message & message )
{
	std::vector<t_message> replies;
	std::vector<std::string> reply_params;
	Client * client = this->_current_client;

	if (client->isRegistered() == false)
	{
		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
		return replies;
	}
	if (message.params.size() > 0 && message.params[0] != this->getName())
		replies.push_back(createReply(ERR_NOSUCHSERVER, ERR_NOSUCHSERVER_STR, message.params[0]));
	else
	{
		reply_params.push_back(this->getName());
		reply_params.push_back(cmdTimeTimestamp());
		replies.push_back(createReply(RPL_TIME, RPL_TIME_STR, reply_params));
	}
	return replies;
}
