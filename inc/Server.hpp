#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <map>
#include <sys/socket.h> // struct sockaddr_in
#include <sys/poll.h> // struct pollfd

#include "Client.hpp"

typedef struct s_message {
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
	bool valid;
	// apresas-: More info might be needed here later
}				t_message;

#define SERVER_NAME_MAX_LENGTH 63

class Server {
	private:
		// uint16_t			_port; // apresas-: in case we need it as a number
		std::string			_port;
		std::string			_password;
		int					_serverFd;
		unsigned int		_client_count;
		struct sockaddr_in	_serverAddress;
		struct pollfd		_pollFds[MAX_CLIENTS + 1];

		bool				_running; // apresas-: Maybe

		static Server		*instance;

		// apresas-: This is a map of function pointers for command functions
		// For now it's only an idea, it might be discarded
		std::map<std::string, void (Server::*)(t_message &)>	_commandMap;

		std::map<int, Client>	_clients;

		void parseInput( void );
		void initServer( void );

		void createSocket( void );
		void bindSocket( void );
		void configureListening( void );
		void runServerLoop( void );
		void handleNewConnections( void );
		void newClient( void );
		void handleClientData( void );
		void getClientData( int i ); // apresas-: New idea
		void sendData( const char *message );
		void setNonBlock( int & socketFd );
		void cleanClose( void );

		void parseData( const std::string & message );
		void runCommand( t_message & message );

		// Commands
		void	cmdPass( t_message & message );
		void	cmdNick( t_message & message );
		void	cmdUser( t_message & message );

		static void signalHandler( int signal );

	public:
		Server( const std::string & port, const std::string & password );
		~Server();

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
			private:
				std::string message;
		};
		class SetsockoptException : public std::exception {
			public:
				SetsockoptException( const char * str ) : message("setsockopt failed to set (" + std::string(str) + ") socket options.") {}
			virtual const char * what() const throw() {
				return message.c_str();
			}
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
