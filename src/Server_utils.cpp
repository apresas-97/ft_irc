#include "ft_irc.hpp"
#include "Server.hpp"

bool Server::isUserInServer( const std::string & nickname ) 
{
	Client * client = findClient(nickname);
	if (client)
		return true;
	return false;
}

Client * Server::findClient( int fd ) 
{
	std::map<int, Client>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return (NULL); // apresas-: Maybe? I'd rather use references but I'm not sure how to handle this rn
	return &it->second;
}

Client * Server::findClient( const std::string & nickname ) 
{
	std::map<std::string, int>::iterator it = this->_clients_fd_map.find(nickname);
	if (it == this->_clients_fd_map.end())
		return (NULL);
	return findClient(it->second);
}

bool Server::isChannelInServer( const std::string & name ) 
{
	Channel * channel = findChannel(name);
	if (channel)
		return true;
	return false;
}

Channel * Server::findChannel( const std::string & name ) 
{
	std::map<std::string, Channel *>::iterator it = this->_channels.find(name);
	if (it == this->_channels.end())
		return (NULL);
	return it->second;
}

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
	std::cout << "t_message: ";
	std::cout << "Prefix [" << message.prefix << "] ";
	std::cout << "Command [" << message.command << "] ";
	std::cout << "Params ";
	for (size_t i = 0; i < message.params.size(); i++)
		std::cout << "[" << message.params[i] << "] ";
	std::cout << std::endl;
	std::cout << "Sender: " << message.sender_client_fd << std::endl;
	std::cout << "Target: " << message.target_client_fd << std::endl;
	std::cout << std::endl;
}
