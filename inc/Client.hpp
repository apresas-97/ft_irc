#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <sys/socket.h> // struct sockaddr_in
#include <sys/poll.h> // struct pollfd

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
		Client( int fd, struct sockaddr_storage address );
		~Client( void );

		void	sendData( const char * message );
		void	getClientData( void );

		const std::string &	getNickname( void ) const;
		const std::string &	getUsername( void ) const;
		const std::string &	getHostname( void ) const;
		const std::string &	getRealname( void ) const;

		void setNickname( const std::string & nickname );
		void setUsername( const std::string & username );
		void setHostname( const std::string & hostname );
		void setRealname( const std::string & realname );

		std::string	getPrefix( void ) const;
		std::string	getUserIdentifier( void ) const;

		bool	isAuthorised( void ) const;
		void	setAuthorised( bool value );

		void	setRegistered( bool value );
		bool	isRegistered( void ) const;

		bool	getMode( char mode ) const;
		void	setMode( char mode, bool value );

	private:

		std::string	_nickname;
		std::string	_username;
		std::string	_hostname;
		std::string	_realname;

		t_mode _mode;

		struct sockaddr_storage _address; // apresas-: Does the client need this?

		bool	_authorised; // Has the client provided the correct password? (PASS command)
		bool	_registered; // Has the client properly registered as a user? (NICK and USER commands)

};

#endif // CLIENT_HPP