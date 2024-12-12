#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sstream>
#include <cerrno>
#include <exception>
#include <cstdlib>
#include <sys/types.h>
#include <poll.h>
#include <cstring>
#include <ctime>
#include <iomanip>

#include "ft_irc.hpp"
#include "Server.hpp"

#define PORT 8080 // Port number to bind

Server*	Server::instance = NULL;

Server::Server( const std::string & port, const std::string & password ) : _port(port), _password(password) 
{
	instance = this;
	setVersion(1, 0);
	setStartTime();
	parseInput();
	initServer();
}

Server::~Server( void ) 
{
	if (_serverFd != -1) 
	{
		std::cout << "Server destructor called" << std::endl;
		if (close(_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
	}
}

void Server::signalHandler( int signal ) 
{
	if (signal == SIGINT) 
	{
		if (instance)
			instance->cleanClose();
		exit(EXIT_SUCCESS);
	}
}

void Server::cleanClose( void ) 
{
	std::cout << "call clean close" << std::endl;
	if (close(_serverFd) == -1)
		closeFailureLog("serverFd", this->_serverFd);
	for (size_t i = 1; i < MAX_CLIENTS; i++) 
	{
		if (_poll_fds[i].fd == -1)
			continue;
		if (close(_poll_fds[i].fd) == -1)
			closeFailureLog("_poll_fds", i, this->_serverFd);
	}
}

void Server::parseInput( void ) 
{
	unsigned int		port;
	std::istringstream	iss(this->_port);

	iss >> port;
	if (this->_port.empty() || iss.fail() || !iss.eof())
		throw InvalidArgument("Invalid port input received", this->_port);
	if (port < 1024 || port > 65535) // apresas-: ? Need to verify the port range
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
		cleanClose();
		throw std::runtime_error("fcntl failed to get socket flags");
	}
	flags |= O_NONBLOCK;
	if (fcntl(socketFd, F_SETFL, flags) == -1 )
	{
		cleanClose();
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

void Server::runServerLoop( void ) 
{
    _poll_fds[0].fd = _serverFd;
    _poll_fds[0].events = POLLIN;

    for (size_t i = 1; i < MAX_CLIENTS + 1; i++) 
	{
        _poll_fds[i].fd = -1;
        _poll_fds[i].events = POLLIN;
    }
	_client_count = 0;
    std::cout << "Server started, waiting for clients..." << std::endl;

    while (true)
	{
        int pollCount = poll(_poll_fds, _client_count + 1, TIMEOUT);
        if (pollCount < 0)
		{
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }
		else if (pollCount == 0)
		{
        	std::cout << "Poll timed out, no activity" << std::endl;
            continue;
        }
		for (size_t i = 0; i < this->_client_count + 1; i++) 
		{
			if (this->_poll_fds[i].revents & POLLIN)
			{
				if (this->_poll_fds[i].fd == this->_serverFd)
					newClient();
				else
					getClientData( i );
			}
		}
    }
}

void	Server::getClientData( int i ) 
{
	char	buffer[BUFFER_SIZE];
	int		bytes_received = recv(this->_poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received < 0) 
	{
		std::cerr << "Error receiving data from client " << this->_poll_fds[i].fd << std::endl;
	}
	else if (bytes_received == 0)
	{
		std::cout << "Client disconnected: " << this->_poll_fds[i].fd << std::endl;
		if (close(this->_poll_fds[i].fd) == -1) 
		{
			closeFailureLog("_poll_fds", i, this->_poll_fds[i].fd);
			cleanClose();
		}
		this->_poll_fds[i].fd = -1;
		this->_client_count--;
	}
	else 
	{
		/* apresas-:
			TODO:
			
			The data received must be sepparated by CR-LF "\r\n", that's the message delimiter
			
			We have to, get all the messages from the received bytes, parsing them, executing them and sending appropriate responses

			Then, we might have incomplete messages at the end of the buffer, so we must store those leftovers in a buffer
			for that client, so that next time we receive data from that same client, we can append it to their buffer and
			complete the message and continue.

			Issues:
				1. This sounds like a pain to implement
				2. The client might take too long to send the rest of the message, so we should have a time out to discard
					the client's buffer and start fresh next time we receive data from them
					-This sounds like a pain to implement too
			
			Potential workaround:
				- We could limit the amount of data we allow a client to send at once, so that we can be sure we can
					process it all in one go
				- I mean, this is also a security feature, we wouldn't want that kind of exploit of allowing someone to
				send us 1TB of data in one go and crash our server
				- I guess, in a way, this is what we do by default by not handling a client buffer, so we could just handle
				the given buffer and discard incomplete messages.
				Example:
					"PRIVMSG user :Hello\r\nPRIVMSG user2 :Hello2\r\nPRIVMSG user3 :Hello3\r\nPRIVMSG user4 :H"
				In this cse, in our buffer we were only able to fit the first 3 messages, the remaining "PRIVMSG user4 :H"
				excedes our BUFFER_SIZE and is not completed with \r\n, it will be ignored and discarded.
			
			TODO:
				'\0' characters are not allowed in messages
				If a message contains a '\0' character, the message will be silently ignored.
		*/
		buffer[bytes_received] = '\0'; // Maybe this overwrites the last character received from recv, but I'm not sure
		std::cout << "Received from client " << this->_poll_fds[i].fd << ": " << buffer;

		// apresas-: Here is where we have to parse the received data and prepare the response

		std::string message(buffer, strlen(buffer) - 1); // apresas-: Maybe just message(buffer); ?
		std::string response;
		parseData(message, this->_poll_fds[i].fd);

		sendData(buffer);
	}
}

/// apresas-: WIP
void Server::parseData( const std::string & raw_message, int client_fd )
{
	/*
	Format of a valid message:

		[<prefix>] SPACE <command> SPACE [ <argument> *( SPACE <argument> ) ]

		> The prefix is optional and MUST start with a colon ':' to differenciate it from the command
			It is mostly used for server-server communication, but, users can also use it.
			However, for users, as far as I know, the prefix will simply be ignored if it doesn't match
			to the user sending the message.
			HOWEVER
				The server will actually treat the message when sending it forward to other clients and
				it will put in place the CORRECT prefix for the user that sent the message.
				Example:
					I send:
						:NONSENSE PRIVMSG #channel :Hello channel!
					The server will send to other clients:
						:mynickname!myusername@myhostname PRIVMSG #channel :Hello channel!
					
					This is so the receivers of the message know who sent it.
				
			SUMMARY: We ignore the user received prefix, but we will still need to put the correct prefix, either now
			or later. We'll see

		> The command is 1 word and MUST be present.
			If no command is present, the message will be "silently ignored", meaning no response, no error, no nothing.

		> The parameters are optional, some have and some don't
		> There can only be AT MOST 15 parameters in a message, any further parameters will be silently ignored

		> If the LAST argument contains SPACES, it must start with a colon ':' to know not to keep splitting
			Example:
				PRIVMSG #channel :Hello channel! What's up everyone? This is a text messsage with spaces.
				The last argument will be:
				<:Hello channel! What's up everyone? This is a text messsage with spaces.>
		
		> Empty messages will be silently ignored

		> All messages will end with a CRLF (Carriage Return, Line Feed) '\r\n'
			But at this point, that should have already been handled when receiving the message

		> Messages have a max length of 512 bytes, including the CRLF at the end

	EXAMPLE OF A VALID MESSAGE:

		misco!~apresas-@whatever.com PRIVMSG #channel :Hello channel!

		This translates to:

		Prefix: misco!~apresas-@whatever.com (Can be ignored)
		Command: PRIVMSG
		Arguments: <#channel>, <Hello channel!>
			The last argument contains spaces because it is prefixed by ':'
	
	ABOUT SPACES:
		The IRC protocol says each element in a message should be sepparated by 1 SPACE, but from my testing
		It seems servers accept multiple spaces too and they just treat them as 1
	*/

	this->_current_client = &this->_clients[client_fd];
	t_message	message = prepareMessage(raw_message);
	message.sender_client_fd = client_fd;

	if (message.command.empty()) 
	{
		std::cerr << "Empty command received, message will be silently ignored" << std::endl;
		return ;
	}
	std::vector<t_message> replies = runCommand(message);

	// apresas-: At this point, the replies should be ready to be processed back to raw data and sent back to the client
	// For now this will only work for commands that will be returned to the sender
	// Stil need to implement putting the fd of the target of the message in the t_message struct
}

/*
apresas-: WIP, I will at some point make a map of function pointers with their names as keys to avoid the
if-else chain
*/
std::vector<t_message>	Server::runCommand( t_message & message ) 
{
	std::vector<t_message> replies;
	if (message.command == "PASS")
		return this->cmdPass(message);
	else if (message.command == "NICK")
		return this->cmdNick(message);
	else if (message.command == "USER")
		return this->cmdUser(message);
	else if (message.command == "MODE")
		return this->cmdMode(message);
	else
		replies.push_back(createReply(ERR_UNKNOWNCOMMAND, ERR_UNKNOWNCOMMAND_STR, message.command));
	return replies;
}

/*
	Gets the raw message and orders it into a t_message struct
*/
t_message	Server::prepareMessage( std::string raw_message ) 
{
	t_message message;
	std::string word;
	std::istringstream iss(raw_message);
	size_t parameters = 0;

	if (raw_message.empty()) 
	{
		std::cerr << "Empty messages should be silently ignored" << std::endl;
		return message;
	}
	if (raw_message[0] == ':') 
	{ // Get the prefix, if present
		iss >> word;
		message.prefix = word;
	}
	if (!(iss >> word)) 
	{ // Get the commmand
		std::cerr << "Invalid message format, missing command" << std::endl; // Must handle this some way
		return message;
	}
	message.command = word;
	while (iss >> word) 
	{ // Get the parameters, if present
		if (parameters == 15) 
		{
			std::cerr << "Too many parameters in the message, further parameters will be simply ignored" << std::endl;
			break;
		}
		if (word[0] == ':') 
		{
			std::string rest;
			std::getline(iss, rest);
			word += rest;
			message.params.push_back(word);
			parameters++;
			break;
		}
		message.params.push_back(word);
		parameters++;
	}
	if (iss.bad()) 
	{
		std::cerr << "Error reading from the input stream." << std::endl;
		// apresas-: Idek what we should do here or how this could happen exactly
		return message;
	}
	return message;
}

void Server::newClient( void ) 
{
	struct sockaddr_storage	clientAddress;
	socklen_t	addressLen = sizeof(clientAddress);
	int	clientFd = accept(_serverFd, (struct sockaddr *)&clientAddress, &addressLen);
	if (clientFd < 0) 
	{
		std::cerr << "Failed to accept new client" << std::endl;
		return;
	}

	// Verify if we can add the client
	if (this->_client_count == MAX_CLIENTS ) 
	{
		std::cerr << "Max clients reached, closing connection" << std::endl;
		if (close(clientFd) == -1)
		{
			closeFailureLog("clientFd", clientFd);
			cleanClose();
		}
		return;
	}

	// Add the client's fd and events to the pollfd array
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++)
	{
		if (this->_poll_fds[i].fd == -1) 
		{
			this->_poll_fds[i].fd = clientFd;
			// this->_poll_fds[i].events = POLLIN;
			break;
		}
	}

	// Add the client to the _clients map using its fd as the key
	// I wrote 3 methods because I'm not sure if they will really work as intended or compile with std=c++98
	// method 1:
	// std::pair<int, Client>	new_client(clientFd, Client(clientFd, clientAddress));
	// this->_clients.insert(new_client);
	// method 2:
	// this->_clients.insert(std::make_pair(clientFd, Client(clientFd, clientAddress)));
	// method 3:
	this->_clients.insert(std::pair<int, Client>(clientFd, Client(clientFd)));
	this->_client_count++;
}

void Server::sendData(const char *message) 
{
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++) 
		if (_poll_fds[i].fd != -1) 
			send(_poll_fds[i].fd, message, strlen(message), 0);
}

std::string Server::getName( void ) const 
{
	return this->_name;
}

void Server::setVersion( size_t major, size_t minor ) 
{
	this->_version_major = major;
	this->_version_minor = minor;
}

std::string Server::getVersion( void ) const 
{
	std::ostringstream oss;

	oss << this->_version_major;
	oss << ".";
	oss << this->_version_minor;
	std::string version = oss.str();
	return version;
}

void Server::setStartTime( void ) 
{
	this->_start_time = std::time(0);
	std::tm *now = std::localtime(&this->_start_time);

	std::ostringstream oss;
	switch (now->tm_wday) 
	{
		case 0:
			oss << "Sun, ";
			break;
		case 1:
			oss << "Mon, ";
			break;
		case 2:
			oss << "Tue, ";
			break;
		case 3:
			oss << "Wed, ";
			break;
		case 4:
			oss << "Thu, ";
			break;
		case 5:
			oss << "Fri, ";
			break;
		case 6:
			oss << "Sat, ";
			break;
	}
	switch (now->tm_mon) 
	{
		case 0:
			oss << "Jan ";
			break;
		case 1:
			oss << "Feb ";
			break;
		case 2:
			oss << "Mar ";
			break;
		case 3:
			oss << "Apr ";
			break;
		case 4:
			oss << "May ";
			break;
		case 5:
			oss << "Jun ";
			break;
		case 6:
			oss << "Jul ";
			break;
		case 7:
			oss << "Aug ";
			break;
		case 8:
			oss << "Sep ";
			break;
		case 9:
			oss << "Oct ";
			break;
		case 10:
			oss << "Nov ";
			break;
		case 11:
			oss << "Dec ";
			break;
	}
	oss << std::setw(2) << std::setfill('0') << now->tm_mday << "at ";
	oss << std::setw(2) << std::setfill('0') << now->tm_hour + 1 << ":";
	oss << std::setw(2) << std::setfill('0') << now->tm_min << ":";
	oss << std::setw(2) << std::setfill('0') << now->tm_sec << " UTC";

	this->_start_time_str = oss.str();
}

std::string Server::getStartTimeStr( void ) 
{
	return this->_start_time_str;
}

bool Server::isUserInServer( const std::string & nickname ) 
{
	Client * client = findClient(nickname);
	if (client)
		return true;
	return false;
}

Client * Server::findClient( int fd ) 
{
	std::map<int, Client>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return (NULL); // apresas-: Maybe? I'd rather use references but I'm not sure how to handle this rn
	return &it->second;
}

Client * Server::findClient( const std::string & nickname ) 
{
	std::map<std::string, int>::iterator it = this->_clients_fd_map.find(nickname);
	if (it == this->_clients_fd_map.end())
		return (NULL);
	return findClient(it->second);
}

bool Server::isChannelInServer( const std::string & name ) 
{
	Channel * channel = findChannel(name);
	if (channel)
		return true;
	return false;
}

Channel * Server::findChannel( const std::string & name ) 
{
	std::map<std::string, Channel>::iterator it = this->_channels.find(name);
	if (it == this->_channels.end())
		return (NULL);
	return &it->second;
}

