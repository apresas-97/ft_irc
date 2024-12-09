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
		Client( int fd, struct sockaddr_in address );
		~Client( void );

		void	sendData( const char * message );
		void	getClientData( void );

		// apresas-: Maybe we just need one version of this, whichever is more comfortable to use
		void	setMode( bool value, char mode );
		void	setMode( const std::string & mode );
		void	setMode( void ); // default mode values

	private:

		// struct sockaddr_in	_address; // apresas-: Does the client need this?
		struct sockaddr_storage _address; // apresas-: Does the client need this?

		bool	_registered; // Has the client properly registered as a user? (NICK and USER commands)
		bool	_authorised; // Has the client provided the correct password? (PASS command)

};

#endif // CLIENT_HPP