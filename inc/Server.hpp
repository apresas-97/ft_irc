#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <sys/socket.h> // struct sockaddr_in
#include <sys/poll.h> // struct pollfd

#include "ft_irc.hpp"
#include "irc_ctype.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "reply_codes.hpp"

#include "defines.hpp"

typedef struct s_message 
{
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
	int sender_client_fd; // fd of the client that sent the message
	int target_client_fd; // fd of the client that is the target of the message
	std::set<int> target_client_fds; // fd of the clients that are the targets of the message // NEW
	std::vector<Channel *>	target_channels;
	// apresas-: More info might be needed here later
}				t_message;

class Server 
{
	private:
		// uint16_t			_port; // apresas-: in case we need it as a number
		std::string			_port;
		std::string			_password;
		int					_serverFd;
		
		unsigned int		_client_count;
		struct sockaddr_in	_server_address;
		std::vector<struct pollfd>	_poll_fds;

		std::string			_name;
		size_t				_version_major;
		size_t				_version_minor;

		time_t				_start_time;
		std::string			_start_time_str;

		bool				_running; // apresas-: Maybe

		static Server		*instance;

		// apresas-: This is a map of function pointers for command functions
		// For now it's only an idea, it might be discarded
		std::map<std::string, void (Server::*)(t_message &)>	_commandMap;

		std::map<int, Client> _clients;
		std::map<std::string, int> _clients_fd_map; // To get the fd of a client by its nickname (not implemented)
		Client * _current_client; // apresas-: Added this, maybe provisionally, for the client that is currently relevant

		std::map<std::string, Channel *> _channels;
		Channel * _current_channel; // apresas-: Added this, maybe provisionally, for the channel that is currently relevant

		std::vector<std::string>	_taken_nicknames;

		void parseInput( void );
		void initServer( void );

		void setVersion( size_t major, size_t minor );
		std::string getVersion( void ) const;

		void setStartTime( void );
		std::string getStartTimeStr( void );

		bool isUserInServer( const std::string & nickname );
		bool isChannelInServer( const std::string & channel );

		Client * findClient( int fd );
		Client * findClient( const std::string & nickname );
		Channel * findChannel( const std::string & name );

		void createSocket( void );
		void bindSocket( void );
		void configureListening( void );
		void runServerLoop( void );
		void newClient( void );
		void getClientData( int i );
		void sendData( const char *message );
		void setNonBlock( int & socketFd );
		void cleanClose( void );
		void removeClient( int fd );

		void parseData( const std::string & message, int client_fd );
		std::vector<t_message> runCommand( t_message & message );

		std::string	getName( void ) const;

		t_message createReply( int number, const std::string message );
		t_message createReply( int number, const std::string message, const std::string & param );
		t_message createReply( int number, const std::string message, const std::vector<std::string> & param );
		t_message createReply( t_message & message, std::string corrected_param, std::string nickname );
		t_message replyList(Client *client, Channel *channel, std::vector<int>& fds);

		std::vector<t_message>	createWelcomeReplies( Client * client );

		// Commands
		std::vector<t_message> cmdInvite( t_message & message );
		std::vector<t_message> cmdJoin( t_message & message );
		// std::vector<t_message> cmdKick( t_message & message );
		std::vector<t_message> cmdMode( t_message & message );
		std::vector<t_message> cmdNick( t_message & message );
		std::vector<t_message> cmdPass( t_message & message );
		std::vector<t_message> cmdPrivMsg( t_message & message );
		std::vector<t_message> cmdQuit( t_message & message );
		std::vector<t_message> cmdTopic( t_message & message );
		std::vector<t_message> cmdUser( t_message & message );
		std::vector<t_message> cmdChanMode( t_message & message, t_mode modes ); 

		t_message	prepareMessage( std::string rawMessage );

		static void signalHandler( int signal );
	
		// Utils
		bool	hasNULL( const char * buffer, int bytes_received ) const;
		bool	hasCRLF( const std::string ) const;
		std::string	stringToUpper( std::string src );

		// DEBUG
		void	printTmessage( t_message message ) const;

	public:
		Server( const std::string & port, const std::string & password );
		~Server( void );
	private:
		// apresas-: I leave this here for now
		void closeFailureLog( const std::string & fd_name, int fd ) 
		{
			std::cerr << "Error: Failed to close file descriptor: " << fd_name << " (" << fd << ")" << std::endl;
		}
		// apresas-: For pollFd[i]
		void closeFailureLog( const std::string & fd_name, int i, int fd ) 
		{
			std::cerr << "Error: Failed to close file descriptor: " << fd_name << "[" << i << "]" << " (" << fd << ")" << std::endl;
		}

		class InvalidArgument : public std::exception 
		{
			public:
				InvalidArgument( const char * str, const char * arg ) : message(std::string(str) + " => \"" + std::string(arg) + "\"")  {}
				InvalidArgument( const char * str, const std::string & arg ) : message(std::string(str) + " => \"" + arg + "\"")  {}
				virtual const char * what() const throw()
				{
					return message.c_str();
				}
				virtual ~InvalidArgument() throw() {}
			private:
				std::string message;
		};
};

#endif // SERVER_HPP

