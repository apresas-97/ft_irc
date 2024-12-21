#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <cerrno>
#include <exception>
#include <cstdlib>
#include <sys/types.h>
#include <poll.h>
#include <cstring>
#include <ctime>
#include <iomanip>

#include "ft_irc.hpp"
#include "Server.hpp"

bool	Server::hasNULL( const char * buffer, int bytes_received ) const
{
	for (int i = 0; i < bytes_received; i++)
		if (buffer[i] == '\0')
			return true;
	return false;
}

bool	Server::hasCRLF( const std::string str ) const
{
	if (str.size() > 1)
		return str[str.size() - 2] == '\r' && str[str.size() - 1] == '\n';
	return false;
}

std::string	Server::stringToUpper( std::string src )
{
	std::string str;
	for (std::string::iterator it = src.begin(); it != src.end(); it++)
		str += toupper(*it);
	return str;
}

void	Server::printTmessage( t_message message ) const 
{
	std::cout << "Prefix [" << message.prefix << "] ";
	std::cout << "Command [" << message.command << "] ";
	std::cout << "Params ";
	for (size_t i = 0; i < message.params.size(); i++)
		std::cout << "[" << message.params[i] << "] ";
	std::cout << "Sender: " << message.sender_client_fd << std::endl;
	std::cout << "Target: " << message.target_client_fd << std::endl;
	std::cout << std::endl;
}
