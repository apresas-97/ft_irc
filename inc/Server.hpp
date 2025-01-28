#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <cstring>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <sys/socket.h> // struct sockaddr_in
#include <sys/poll.h> // struct pollfd

#include "ft_irc.hpp"
#include "utils.hpp"
#include "irc_ctype.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "reply_codes.hpp"

#include "defines.hpp"

typedef struct s_message 
{
	s_message() : prefix(""), command(""), sender_client_fd(-1) {}
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
	int sender_client_fd; // fd of the client that sent the message
	std::set<int> target_client_fds; // fd of the clients that are the targets of the message // NEW
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

		size_t				_unique_id; // for unique nickname generation

		static Server		*instance;

		std::map<int, Client> _clients;
		std::map<std::string, int> _clients_fd_map; // To get the fd of a client by its nickname (not implemented)
		Client * _current_client; // apresas-: Added this, maybe provisionally, for the client that is currently relevant

		std::map<std::string, Channel> _channels;
		Channel * _current_channel; // apresas-: Added this, maybe provisionally, for the channel that is currently relevant

		std::vector<std::string>	_taken_nicknames;

		void parseInput( void );
		void initServer( void );

		void setName( const std::string & name );

		void setVersion( size_t major, size_t minor );
		std::string getVersion( void ) const;

		void setStartTime( void );
		std::string getStartTimeStr( void );

		Channel * channelGet( const std::string & name );
		void addChannel(Channel &channel, std::string &name);
		bool channelFound(const std::string& chanName);
		bool isUserInServer( const std::string & nickname );
		bool isChannelInServer( const std::string & channel );
		void uninviteUser( std::string nickname );

		void removeChannel( const std::string &name );

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
		void cleanClose( bool );
		void removeClient( int fd );

		std::string	getName( void ) const;

		// Data
		t_message parseData( const std::string & raw_message, int client_fd );

		std::vector<t_message> runCommand( t_message & message );
		// Commands
		std::vector<t_message> cmdInvite( t_message & message );
		std::vector<t_message> cmdJoin( t_message & message );
		std::vector<t_message> cmdKick( t_message & message );
		std::vector<t_message> cmdMode( t_message & message );
		std::vector<t_message> cmdModeUser( t_message & message );
		std::vector<t_message> cmdModeChannel( t_message & message );
		std::vector<t_message> cmdNick( t_message & message );
		std::vector<t_message> cmdPass( t_message & message );
		std::vector<t_message> cmdPrivMsg( t_message & message );
		std::vector<t_message> cmdNotice( t_message & message );
		std::vector<t_message> cmdQuit( t_message & message );
		std::vector<t_message> cmdTopic( t_message & message );
		std::vector<t_message> cmdUser( t_message & message );
		std::vector<t_message> cmdError( std::string & error_message );
		std::vector<t_message> cmdVersion( t_message & message );
		std::vector<t_message> cmdTime( t_message & message );
		std::vector<t_message> cmdPing( t_message & message );
		std::vector<t_message> cmdPong( t_message & message );
		std::vector<t_message> cmdPart( t_message & message );
		std::vector<t_message> cmdMotd( t_message & message );
		std::vector<t_message> cmdNames( t_message & message );
		std::vector<t_message> cmdInfo( t_message & message );
		std::vector<t_message> cmdList( t_message & message );
		std::vector<t_message> cmdWho( t_message & message );
		
		// Messages
		t_message	prepareMessage( std::string rawMessage );
		std::vector<std::string> splitMessage( std::string & message );

		// Replies
		void		sendReply( t_message reply );
		std::string	formatReply( t_message reply );

		t_message createReply( int number, const std::string message );
		t_message createReply( int number, const std::string message, const std::string & param );
		t_message createReply( int number, const std::string message, const std::vector<std::string> & param );
		t_message createReply( t_message & message, std::string corrected_param, std::string nickname );
		t_message replyList(Client *client, Channel *channel, std::vector<int>& fds);

		std::vector<t_message>	createWelcomeReplies( Client * client );

		// Utils
		bool	hasNULL( const char * buffer, int bytes_received ) const;
		bool	hasCRLF( const std::string ) const;
		std::string	stringToUpper( std::string src );
		std::string getTimestamp( time_t time );
		void	addChannelToReply( t_message &, Channel * );
		void	addChannelToReplyExcept( t_message &, Channel * );
		void	addUserToChannel( std::string channel_name, Client * client, bool as_operator );
		t_message	createNotice( Client * client, const std::string & message );
		void	setupClientHostname( Client & client );
		bool	isNicknameTaken( const std::string & nickname ) const;
		void	removeTakenNickname( const std::string & nickname );
		void	replaceTakenNickname( Client * client, const std::string & new_nickname );
		void	updateClientNickname( Client * client, const std::string & new_nickname );
		std::string generateUniqueNickname( void );

		// IDK
		void	checkInactivity( void );
		void	removeTerminatedClients( void );
		static void signalHandler( int signal );

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

