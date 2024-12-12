#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <sys/socket.h> // struct sockaddr_in
#include <sys/poll.h> // struct pollfd

#include "ft_irc.hpp"
#include "irc_ctype.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "reply_codes.hpp"

// Limits
#define MAX_CLIENTS 10
#define MAX_CLIENT_NICKNAME_LENGTH 9
#define MAX_CLIENT_USERNAME_LENGTH 32
#define MAX_CLIENT_REALNAME_LENGTH 100

#define SERVER_NAME_MAX_LENGTH 63

#define	TIMEOUT		5000
#define	BUFFER_SIZE 1024

#define USER_MODES "aiwroOs"
#define CHANNEL_MODES "itkol"

typedef struct s_message {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
	int sender_client_fd; // fd of the client that sent the message
	int target_client_fd; // fd of the client that is the target of the message
	// apresas-: More info might be needed here later
}				t_message;

class Server {
	private:
		// uint16_t			_port; // apresas-: in case we need it as a number
		std::string			_port;
		std::string			_password;
		int					_serverFd;
		
		unsigned int		_client_count;
		struct sockaddr_in	_server_address;
		struct pollfd		_poll_fds[MAX_CLIENTS + 1];

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

		std::map<std::string, Channel> _channels;
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
		void getClientData( int i ); // apresas-: New idea
		void sendData( const char *message );
		void setNonBlock( int & socketFd );
		void cleanClose( void );

		void parseData( const std::string & message, int client_fd );
		std::vector<t_message> runCommand( t_message & message );

		std::string	getName( void ) const;

		t_message createReply( int number, const std::string message );
		t_message createReply( int number, const std::string message, const std::string & param );
		t_message createReply( int number, const std::string message, const std::vector<std::string> & param );
		t_message createReply( t_message & message, std::string corrected_param, std::string nickname );

		// Commands
		std::vector<t_message> cmdPass( t_message & message );
		std::vector<t_message> cmdNick( t_message & message );
		std::vector<t_message> cmdUser( t_message & message );
		std::vector<t_message> cmdMode( t_message & message );

		t_message	prepareMessage( std::string rawMessage );

		static void signalHandler( int signal );

	public:
		Server( const std::string & port, const std::string & password );
		~Server( void );
	private:
		// apresas-: I leave this here for now
		void closeFailureLog( const std::string & fd_name, int fd ) {
			std::cerr << "Error: Failed to close file descriptor: " << fd_name << " (" << fd << ")" << std::endl;
		}
		// apresas-: For pollFd[i]
		void closeFailureLog( const std::string & fd_name, int i, int fd ) {
			std::cerr << "Error: Failed to close file descriptor: " << fd_name << "[" << i << "]" << " (" << fd << ")" << std::endl;
		}

		// apresas-: I leave this here for now, currently UNUSED
		// class CloseException : public std::exception {
		// 	public:
		// 		CloseException( const char * fd_name, int fd ) : message("failed to close file descriptor: " + std::string(fd_name) + " (" + std::to_string(fd) + ")") {}
		// 		CloseException( const char * fd_name ) : message("failed to close file descriptor: " + std::string(fd_name)) {}
		// 		CloseException( int fd ) : message("failed to close file descriptor: " + std::to_string(fd)) {}
		// 		virtual const char * what() const throw() {
		// 			return message.c_str();
		// 		}
		// 	private:
		// 		std::string message;
		// };
		class InvalidArgument : public std::exception {
			public:
				InvalidArgument( const char * str, const char * arg ) : message(std::string(str) + " => \"" + std::string(arg) + "\"")  {}
				InvalidArgument( const char * str, const std::string & arg ) : message(std::string(str) + " => \"" + arg + "\"")  {}
				virtual const char * what() const throw() {
					return message.c_str();
				}
				virtual ~InvalidArgument() throw() {}
			private:
				std::string message;
		};
		class SetsockoptException : public std::exception {
			public:
				SetsockoptException( const char * str ) : message("setsockopt failed to set (" + std::string(str) + ") socket options.") {}
			virtual const char * what() const throw() {
				return message.c_str();
			}
			virtual ~SetsockoptException() throw() {}
			private:
				std::string message;
		};
		class BindException : public std::exception
		{
			virtual const char * what() const throw()
			{
				return "Server socket bind failed.";
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
