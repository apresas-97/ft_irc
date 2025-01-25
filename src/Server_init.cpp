#include <cstring>
#include "ft_irc.hpp"
#include "Server.hpp"

#define PORT 8080 // Port number to bind

void Server::parseInput( void ) 
{
	unsigned int		port;
	std::istringstream	iss(this->_port);

	iss >> port;
	if (this->_port.empty() || iss.fail() || !iss.eof())
		throw InvalidArgument("Invalid port input received", this->_port);
	if (port < 1025 || port > 65535) // apresas-: ? Need to verify the port range
		throw InvalidArgument("Port must be between 1024 and 49151", this->_port);
}

void Server::initServer( void ) 
{
	if (signal(SIGINT, signalHandler) == SIG_ERR)
		throw std::runtime_error("Failed to set up signal handler");
	createSocket();
	bindSocket();
	configureListening();
	runServerLoop();
}

void Server::createSocket( void ) 
{
	this->_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverFd < 0)
		throw std::runtime_error("Server socket creation failed");

	int opt = 1;
	if (setsockopt(this->_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) 
	{
		if (close(this->_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
		throw std::runtime_error("setsockopt failed to set ( SO_REUSEADDR | SO_REUSEPORT ) socket options");
	}

	setNonBlock(_serverFd);
}

void Server::setNonBlock(int & socketFd) 
{
	int	flags = fcntl( socketFd, F_GETFL, 0 );
	if (flags < 0)
	{
		cleanClose(false);
		throw std::runtime_error("fcntl failed to get socket flags");
	}
	flags |= O_NONBLOCK;
	if (fcntl(socketFd, F_SETFL, flags) == -1 )
	{
		cleanClose(false);
		throw std::runtime_error("fcntl failed to set socket flags");
	}
}

void Server::bindSocket( void ) 
{
	uint16_t			port;
	std::istringstream	iss(this->_port);

	iss >> port;
    memset(&this->_server_address, 0, sizeof(this->_server_address));
    this->_server_address.sin_family = AF_INET;
    this->_server_address.sin_addr.s_addr = INADDR_ANY;
    this->_server_address.sin_port = htons(port);

	if (bind(this->_serverFd, (struct sockaddr *)&this->_server_address, sizeof(this->_server_address)) < 0) 
	{
		if (close(this->_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
		throw std::runtime_error("Server socket bind failed");
	}
}

void Server::configureListening( void ) 
{
    if (listen(this->_serverFd, MAX_CLIENTS) < 0) 
	{
        if (close(this->_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
		throw std::runtime_error("Listen failed");
    }
    std::cout << "Server is listening on port " << _port << std::endl;
}

