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

Server::Server( const std::string & port, const std::string & password ) : _port(port), _password(password) {
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
	_client_count = 0;
    std::cout << "Server started, waiting for clients..." << std::endl;

    while (true) {
        int pollCount = poll(_pollFds, _client_count + 1, TIMEOUT);
        if (pollCount < 0) {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        } else if (pollCount == 0) {
        	std::cout << "Poll timed out, no activity" << std::endl;
            continue;
        }
		
		for (size_t i = 0; i < this->_client_count; i++) {
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
			TO-DO:
			
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
			
			TO-DO:
				'\0' characters are not allowed in messages
				If a message contains a '\0' character, the message will be silently ignored.
		*/
		buffer[bytes_received] = '\0';
		std::cout << "Received from client " << this->_pollFds[i].fd << ": " << buffer;

		// apresas-: Here is where we have to parse the received data and prepare the response

		std::string message(buffer, strlen(buffer) - 1); // apresas-: Maybe just message(buffer); ?
		std::string response;
		parseData(message, this->_pollFds[i].fd);


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
	t_message	message = prepareMessage(raw_message);
	message.sender_client_fd = client_fd;

	// apresas-: Here we can start parsing the message

	if (!message.valid) {
		std::cerr << "Invalid message format, message will be silently ignored, no reply will be sent" << std::endl;
		return; // Need to figure out how to make the program know not to send a reply in this case
	}

	// apresas-: Added this, maybe later there should be a better way of doing this
	message.prefix = this->_clients[client_fd].getPrefix(); // This is actually only necessary when the message must be sent to a client, we could do it later and only if necessary

	runCommand(message);
}

void	Server::runCommand( t_message & message ) {
	/* apresas-:
		Current idea:
		This will use a std::map<std::string, void (Server::*)(t_message &)>
		this way we can get the right function to run based on the command

		Still need to think about this
	*/
	// For now:
	if (message.command == "PASS")
		this->cmdPass(message);
	else if (message.command == "NICK")
		this->cmdNick(message);
	else if (message.command == "USER")
		this->cmdUser(message);
	else
		std::cerr << "Command not recognized, message will be silently ignored" << std::endl;
}

/*
	Gets the raw message and orders it into a t_message struct
*/
t_message	prepareMessage( std::string raw_message ) {
	t_message					message;
	std::string					word;
	std::istringstream			iss(raw_message);
	size_t	parameters = 0;
	message.valid = false;

	if (raw_message.empty()) {
		std::cerr << "Empty messages should be silently ignored" << std::endl;
		return message;
	}
	if (raw_message.front() == ':') { // Get the prefix, if present
		iss >> word;
		message.prefix = word;
	}
	if (!(iss >> word)) { // Get the commmand
		std::cerr << "Invalid message format, missing command" << std::endl; // Must handle this some way
		return message;
	}
	message.command = word;
	while (iss >> word) { // Get the parameters, if present
		if (parameters == 15) {
			std::cerr << "Too many parameters in the message, further parameters will be simply ignored" << std::endl;
			break;
		}
		if (word.front() == ':') {
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
	if (iss.bad()) {
		std::cerr << "Error reading from the input stream." << std::endl;
		// apresas-: Idek what we should do here or how this could happen exactly
		return message;
	}
	message.valid = true;
	return message;
}

void Server::newClient( void ) {
	struct sockaddr_storage	clientAddress;
	socklen_t	addressLen = sizeof(clientAddress);
	int	clientFd = accept(_serverFd, (struct sockaddr *)&clientAddress, &addressLen);
	if (clientFd < 0) {
		std::cerr << "Failed to accept new client" << std::endl;
		return;
	}

	// Verify if we can add the client
	if (this->_client_count == MAX_CLIENTS ) { // apresas-: Maybe do this???
		std::cerr << "Max clients reached, closing connection" << std::endl;
		if (close(clientFd) == -1) {
			closeFailureLog("clientFd", clientFd);
			cleanClose();
		}
		return;
	}

	// Add the client's fd and events to the pollfd array
	this->_pollFds[this->_client_count].fd = clientFd;
	this->_pollFds[this->_client_count].events = POLLIN;
	// Add the client to the _clients map using its fd as the key
	// I wrote 3 methods because I'm not sure if they will really work as intended or compile with std=c++98
	// method 1:
	// std::pair<int, Client>	new_client(clientFd, Client(clientFd, clientAddress));
	// this->_clients.insert(new_client);
	// method 2:
	// this->_clients.insert(std::make_pair(clientFd, Client(clientFd, clientAddress)));
	// method 3:
	this->_clients.insert(std::pair<int, Client>(clientFd, Client(clientFd, clientAddress)));
	this->_client_count++;
}

void Server::sendData(const char *message) {
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
		if (_pollFds[i].fd != -1) {
			send(_pollFds[i].fd, message, strlen(message), 0);
		}
	}
}
