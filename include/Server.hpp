#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <list>

#include "ft_irc.hpp"

//
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
//

#define SERVER_NAME_MAX_LENGTH 63

#define BUFFER_SIZE 1024

class Server
{
// I think there is no longer need to make the class in orthodox canonical form
// private:
	// Server( void ); // No queremos un Server default constructed
	// Server( const Server & src ); // Tampoco nos interesa copiar un Server

public:
	Server( t_port port, const std::string & password );
	~Server();

	Server & operator=( const Server & src );

	int	getFd( void ) const;
	void	initSocket( void );
	void	setSocketOptions( void );
	void	bindSocket( void );
	void	initListen( void );
	void	acceptConnection( void );
	void	receiveData( void );
	void	sendData( const std::string & data );
	void	closeConnection( void );

private:
	// std::string	_name; // server name, max length of 63 characters

	t_port	_port;
	std::string	_password;

	int		_fd;
	int		_client_fd; // Later this will be a list of clients each with its own fd

	// Channels ?
	// std::list<Channel>	_channels;

	// Users ?
	// std::list<User>	_users;
	class SocketCreationException : public std::exception
	{
		virtual const char * what() const throw()
		{
			return "Socket creation failed.";
		}
	};
	class SetsockoptException : public std::exception
	{
		virtual const char * what() const throw()
		{
			return "setsockopt failed.";
		}
	};
	class BindException : public std::exception
	{
		virtual const char * what() const throw()
		{
			return "Bind failed.";
		}
	};
	class ListenFailedException : public std::exception
	{
		virtual const char * what() const throw()
		{
			return "Listen failed.";
		}
	};
	class AcceptFailedException : public std::exception
	{
		virtual const char * what() const throw()
		{
			return "Accept failed.";
		}
	};
	class RecvFailedException : public std::exception
	{
		virtual const char * what() const throw()
		{
			return "Recv failed.";
		}
	};

};


#endif // SERVER_HPP