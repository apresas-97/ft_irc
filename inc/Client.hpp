#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <sys/socket.h> // struct sockaddr_in
#include <sys/poll.h> // struct pollfd

#include "Channel.hpp"

class Channel;

typedef struct s_mode
{
	bool	a; // AWAY 
	bool	i; // INVISIBLE
	bool	w; // WALLOPS
	bool	r; // RESTRICTED
	bool	o; // OPERATOR
	bool	O; // LOCAL OPERATOR
	bool	s; // RECEIVE SERVER NOTICES
}				t_mode;

class Client
{
	public:
		Client( void );
		Client( int socket );
		// Client( int fd, struct sockaddr_storage address );
		~Client( void );

		void	sendData( const char * message );
		void	getClientData( void );

		const std::string &	getNickname( void ) const;
		const std::string &	getUsername( void ) const;
		const std::string &	getHostname( void ) const;
		const std::string &	getRealname( void ) const;
		const int & getSocket( void ) const;

		void setNickname( const std::string & nickname );
		void setUsername( const std::string & username );
		void setHostname( const std::string & hostname );
		void setRealname( const std::string & realname );
		void setSocket( const int & socket );

		std::string	getPrefix( void ) const;
		std::string	getUserIdentifier( void ) const;

		bool	isAuthorised( void ) const;
		void	setAuthorised( bool value );

		bool	isRegistered( void ) const;
		void	setRegistered( bool value );

		bool	getMode( char mode ) const;
		t_mode	getModes( void ) const;
		void	setMode( char mode, bool value );

		bool	hasMode( char mode ) const;
		const std::string getModeString( void ) const ;

	private:

		std::string	_nickname;
		std::string	_username;
		std::string	_hostname;
		std::string	_realname;

		t_mode _mode;

		int		_socket;
		// struct sockaddr_storage _address; // apresas-: Does the client need this?

		std::map<std::string, Channel*>	_channels;

		bool	_authorised; // Has the client provided the correct password? (PASS command)
		bool	_registered; // Has the client properly registered as a user? (NICK and USER commands)

		char *	_buffer[BUFFER_SIZE];
};

#endif // CLIENT_HPP

