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
static std::string	getDate();

std::vector<t_message>	Server::cmdVersion( t_message & message )
{
	std::vector<t_message> replies;
	std::vector<std::string> reply_params;

	std::ostringstream	info;
	std::string	date = getDate();
	info << date << " CLIENTLIMIT=" << MAX_CLIENTS << " MAXCLIENTCHANNELS=" << MAX_CLIENT_CHANNELS \
			<< " USERMODES=" << USER_MODES << " CHANNELMODES=" << CHANNEL_MODES;

	if (message.params.size() > 0 && message.params[0] != this->getName())
		replies.push_back(createReply(ERR_NOSUCHSERVER, ERR_NOSUCHSERVER_STR, message.params[0]));
	else
	{
		reply_params.push_back(this->getVersion());
		reply_params.push_back("");
		reply_params.push_back(this->getName());
		reply_params.push_back(info.str());
		replies.push_back(createReply(RPL_VERSION, RPL_VERSION_STR, reply_params));
	}
	return replies;
}

static std::string	getDate()
{
	// __DATE__ is a predefined macro that expands to the date the source file was compiled
	const char* date = __DATE__;  // Example: "Jan 28 2025"
	
	// We'll extract the month, day, and year from the string
	std::string monthStr(date, 3);  // First three characters are the month (e.g., "Jan")
	int day = atoi(date + 4);	   // The next two characters represent the day (e.g., "28")
	int year = atoi(date + 7);	  // The last four characters represent the year (e.g., "2025")
	
	// Map month name to a numeric value
	std::map<std::string, int> monthMap;
	monthMap["Jan"] = 1;
	monthMap["Feb"] = 2;
	monthMap["Mar"] = 3;
	monthMap["Apr"] = 4;
	monthMap["May"] = 5;
	monthMap["Jun"] = 6;
	monthMap["Jul"] = 7;
	monthMap["Aug"] = 8;
	monthMap["Sep"] = 9;
	monthMap["Oct"] = 10;
	monthMap["Nov"] = 11;
	monthMap["Dec"] = 12;
	
	// Get the numeric value for the month
	int month = monthMap[monthStr];
	
	// Format the date as YYYY/MM/DD
	std::ostringstream formattedDate;
	formattedDate << std::setfill('0') << std::setw(2) << day << "/" << std::setfill('0') \
				<< std::setw(2) << month << "/" << year;
	
	return formattedDate.str();
}
