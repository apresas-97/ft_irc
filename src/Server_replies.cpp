#include "Server.hpp"

std::string	Server::formatReply( t_message reply )
{
	std::string	final_reply;

	if (reply.prefix.size() > 0)
		final_reply += reply.prefix + " ";
	final_reply += reply.command;
	for (std::vector<std::string>::iterator it = reply.params.begin(); it != reply.params.end(); ++it)
	{
		if (!std::isdigit(reply.command[0]) && (it + 1 == reply.params.end()))
		{
			std::string last_param = *it;
			if (last_param.find(' ') != std::string::npos)
				final_reply += " :" + last_param;
			else
				final_reply += " " + last_param;
		}
		else
			final_reply += " " + *it;
	}
	final_reply += "\r\n";
	return final_reply;
}

void	Server::sendReply( t_message reply )
{
	std::string	output = formatReply(reply);

	for (std::set<int>::iterator it = reply.target_client_fds.begin(); it != reply.target_client_fds.end(); ++it)
	{
		send(*it, output.c_str(), output.size(), 0);
	}
}
