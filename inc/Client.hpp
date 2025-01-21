#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sys/socket.h> // struct sockaddr_in
#include <sys/poll.h> // struct pollfd
#include <arpa/inet.h> // inet_ntoa
#include <netdb.h> // gethostbyname
#include <ctime>

#include "defines.hpp"
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
		~Client( void );

		// Setters
		void	setSocket( const int & socket );
		void	setSockaddr( const struct sockaddr *addr );
		void	setNickname( const std::string & nickname );
		void	setUsername( const std::string & username );
		void	setHostname( const std::string & hostname );
		void	setRealname( const std::string & realname );
		void	setAuthorised( bool value );
		void	setRegistered( bool value );
		void	setTerminate( bool value );
		void	setHostnameLookedUp( bool value );
		void	setMode( char mode, bool value );
		void	setLastActivity( void );

		// Getters
		int					getSocket( void );
		const std::string &	getNickname( void ) const;
		const std::string &	getUsername( void ) const;
		const std::string &	getHostname( void ) const;
		const std::string &	getRealname( void ) const;
		const std::string	getUserPrefix(void) const;
		bool				isAuthorised( void ) const;
		bool				isRegistered( void ) const;
		bool				isTerminate( void ) const;
		bool				isHostnameLookedUp( void ) const;
		bool				getMode( char mode ) const;
		t_mode				getModes( void ) const;
		int					getChannelCount(void) const;
        int					getChannelLimit(void) const;
		time_t				getLastActivity( void ) const;

		// Channel Management
		void	addChannel(Channel &channel, std::string& name);
		void	removeChannel(Channel &channel, std::string& name);
		std::vector<Channel>	getChannelsVector( void ) const;

		// Mode Management
		bool	hasMode(char mode) const;
		const std::string getModeString(void) const;

		// Hostname Management
		std::string	hostnameLookup( void );

		// Buffer Management
		bool	fillBuffer( std::string );
		void	clearBuffer( void );
		std::string	getBuffer() const;

	private:

		int						_socket;
		const struct sockaddr	*_addr; // For hostname lookup
		std::string				_nickname;
		std::string				_username;
		std::string				_hostname;
		std::string				_realname;
		bool					_authorised; // Has the client provided the correct password? (PASS command)
		bool					_registered; // Has the client properly registered as a user? (NICK and USER commands)
		bool					_terminate; // Should the client's connection be terminated?
		bool					_hostname_looked_up; // Has the hostname lookup been done?
		t_mode					_mode;

		time_t					_last_activity;
		// apresas-: I'm not sure if this is the best way to handle this

		std::map<std::string, Channel>	_channels;
		int 		_chan_limit;
		int 		_chan_count;

		char 		_buffer[BUFFER_SIZE]; // Idk what this does
};

#endif // CLIENT_HPP

/*
	cc->isAdmin(channel)
*/
