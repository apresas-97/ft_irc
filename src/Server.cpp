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

#include "ft_irc.hpp"
#include "Server.hpp"

#define PORT 8080 // Port number to bind

Server*	Server::instance = NULL;

Server::Server( const std::string & port, const std::string & password ) : _port(port), _password(password){
	instance = this;
	parseInput();
	initServer();
}

Server::~Server( void ) {
	if (_serverFd != -1) {
		std::cout << "Server destructor called" << std::endl;
		if (close(_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
	}
}

void Server::signalHandler( int signal ) {
	if (signal == SIGINT) {
		if (instance)
			instance->cleanClose();
		exit(0);
		/* apresas-:
			Should we use exit here?
			Also, should it be exit(0) ? (EXIT_SUCCESS)
			Or exit(EXIT_FAILURE) ? (1)
		*/
	}
}

void Server::cleanClose( void ) {
	std::cout << "call clean close" << std::endl;
	if (close(_serverFd) == -1)
		closeFailureLog("serverFd", this->_serverFd);
	for (size_t i = 1; i < MAX_CLIENTS; i++) {
		if (_pollFds[i].fd == -1)
			continue;
		if (close(_pollFds[i].fd) == -1)
			closeFailureLog("_pollFds", i, this->_serverFd);
	}
}

void Server::parseInput( void ) {

	unsigned int		port;
	std::string			port_str(this->_port);
	std::istringstream	iss(this->_port);

	iss >> port;
	if (this->_port.empty() || iss.fail() || !iss.eof())
		throw InvalidArgument("Invalid port input received", this->_port);
	if (port < 1024 || port > 65535) // apresas-: ? Need to verify the port range
		throw InvalidArgument("Port must be between 1024 and 49151", this->_port);
}

void Server::initServer( void ) {
	if (signal(SIGINT, signalHandler) == SIG_ERR)
		throw std::runtime_error("Failed to set up signal handler");
	createSocket();
	bindSocket();
	configureListening();
	runServerLoop();
}

void Server::createSocket( void ) {
	this->_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_serverFd < 0)
		throw std::runtime_error("Server socket creation failed");

	int opt = 1;
	if (setsockopt(this->_serverFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
		if (close(this->_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
		throw SetsockoptException("SO_REUSEADDR | SO_REUSEPORT");
	}

	setNonBlock(_serverFd);
}

void Server::setNonBlock(int & socketFd) {
	int	flags = fcntl( socketFd, F_GETFL, 0 );
	if (flags < 0) {
		cleanClose();
		throw std::runtime_error("fcntl failed to get socket flags");
	}
	flags |= O_NONBLOCK;
	if (fcntl(socketFd, F_SETFL, flags) == -1 ) {
		cleanClose();
		throw std::runtime_error("fcntl failed to set socket flags");
	}
}

void Server::bindSocket( void ) {
	uint16_t			port;
	std::istringstream	iss(this->_port);

	iss >> port;
    memset(&this->_serverAddress, 0, sizeof(this->_serverAddress));
    this->_serverAddress.sin_family = AF_INET;
    this->_serverAddress.sin_addr.s_addr = INADDR_ANY;
    this->_serverAddress.sin_port = htons(port);

	if (bind(this->_serverFd, (struct sockaddr *)&this->_serverAddress, sizeof(this->_serverAddress)) < 0) {
		if (close(this->_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
		throw std::runtime_error("Server socket bind failed");
	}
}

void Server::configureListening( void ) {
    if (listen(this->_serverFd, MAX_CLIENTS) < 0) {
        if (close(this->_serverFd) == -1)
			closeFailureLog("serverFd", this->_serverFd);
		throw std::runtime_error("Listen failed");
    }
    std::cout << "Server is listening on port " << _port << std::endl;
}

void Server::runServerLoop( void ) {
    _pollFds[0].fd = _serverFd;
    _pollFds[0].events = POLLIN;

    for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
        _pollFds[i].fd = -1;
        _pollFds[i].events = POLLIN;
    }
	_clients = 0;
    std::cout << "Server started, waiting for clients..." << std::endl;

    while (true) {
        int pollCount = poll(_pollFds, _clients + 1, TIMEOUT);
        if (pollCount < 0) {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        } else if (pollCount == 0) {
        	std::cout << "Poll timed out, no activity" << std::endl;
            continue;
        }
		
		for (size_t i = 0; i < this->_clients; i++) {
			if (this->_pollFds[i].revents & POLLIN) {
				if (this->_pollFds[i].fd == this->_serverFd)
					handleNewConnections();
				else
					handleClientData();
			}
		}
    }
}

// apresas-: New idea
void	Server::getClientData( int i ) {
	char	buffer[BUFFER_SIZE];
	int		bytes_received = recv(this->_pollFds[i].fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes_received < 0) {
		std::cerr << "Error receiving data from client " << this->_pollFds[i].fd << std::endl;
	} else if (bytes_received == 0) {
		std::cout << "Client disconnected: " << this->_pollFds[i].fd << std::endl;
		if (close(this->_pollFds[i].fd) == -1) {
			closeFailureLog("_pollFds", i, this->_pollFds[i].fd);
			cleanClose(); // apresas-: We might have to handle some other things here
		}
		this->_pollFds[i].fd = -1;
		if (static_cast<size_t>(i) == this->_clients) // apresas-: Unsure about this
			this->_clients--;
	} else {
		/* apresas-:
			We need to check, does recv not null-terminate the buffer?
			What if we receive exactly BUFFER_SIZE bytes?
			We would then be overwriting the last byte with a null terminator
		*/
		buffer[bytes_received] = '\0';
		std::cout << "Received from client " << this->_pollFds[i].fd << ": " << buffer;

		// apresas-: Here is where we have to parse the received data and prepare the response

		std::string message(buffer, strlen(buffer) - 1); // apresas-: Maybe just message(buffer); ?
		std::string response;
		this->parseData(message);


		sendData(buffer);
	}
}

/// apresas-: WIP
void Server::parseData( const std::string & message )
{
	/*
	Format of a valid message:

		[<prefix>] SPACE <command> SPACE <argument> *( SPACE <argument> )

		> The prefix is optional and MUST start with a colon ':' to differenciate it from the command
			It is mostly used for server-server communication, but, users can also use it.
			However, for users, as far as I know, the prefix will simply be ignored if it doesn't match
			to the user sending the message.
			So basically, we can check if the first arg starts with ':', we can skip it

		> If the LAST argument contains SPACES, it must start with a colon ':' to know not to keep splitting

	EXAMPLE OF A VALID MESSAGE:

		misco!~apresas-@whatever.com PRIVMSG #channel :Hello channel!

		This translates to:

		Prefix: misco!~apresas-@whatever.com (Can be ignored)
		Command: PRIVMSG
		Arguments: <#channel>, <Hello channel!>
			The last argument contains spaces because it is prefixed by ':'

	*/
	(void)message;
}

///

void Server::handleNewConnections( void ) {
	struct sockaddr_in clientAddress;
	socklen_t addressLen = sizeof(clientAddress);
	int clientFd = accept(_serverFd, (struct sockaddr *)&clientAddress, &addressLen);
	if (clientFd < 0) {
		std::cerr << "Failed to accept new client" << std::endl;
		return;
	}

	bool clientAdded = false;
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
		if (_pollFds[i].fd == -1) {
			_pollFds[i].fd = clientFd;
			std::cout << "New client connected: " << clientFd << std::endl;
			clientAdded = true;
			if (i > _clients)
				_clients++;
			break;
		}
	}

	if (!clientAdded) {
		std::cerr << "Max clients reached, closing connection" << std::endl;
		if (close(clientFd) == -1) {
			closeFailureLog("clientFd", clientFd);
			cleanClose();
		}
	}
}

void Server::handleClientData( void ) {
	char buffer[BUFFER_SIZE];
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
		if (_pollFds[i].fd == -1)
			continue;

		if (_pollFds[i].revents & POLLIN) {
			int bytesReceived = recv(_pollFds[i].fd, buffer, sizeof(buffer) - 1, 0);
			if (bytesReceived < 0) {
				std::cerr << "Error receiving data from client " << _pollFds[i].fd << std::endl;
			} else if (bytesReceived == 0) {
				std::cout << "Client disconnected: " << _pollFds[i].fd << std::endl;
				if (close(_pollFds[i].fd) == -1) {
					closeFailureLog("_pollFds", i, _pollFds[i].fd);
					cleanClose();
				}
				_pollFds[i].fd = -1;
				if (i == _clients)
					_clients--;
			} else {
				buffer[bytesReceived] = '\0';
				std::string message(buffer);
				size_t pos;
				while ((pos = message.find('\n')) != std::string::npos) {
					std::string	fullMsg = message.substr(0, pos);
					std::cout << "Received from client " << _pollFds[i].fd << ": " << buffer;
					message.erase(0, pos + 1);

					sendData(buffer);
				}
				if (!message.empty()) {
					std::cout << "Partial message from client " << _pollFds[i].fd << std::endl;
				}
			}
		}
	}
}

void Server::sendData(const char *message) {
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
		if (_pollFds[i].fd != -1) {
			send(_pollFds[i].fd, message, strlen(message), 0);
		}
	}
}
