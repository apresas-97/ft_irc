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

void Server::runServerLoop( void ) 
{
	struct pollfd	server;

	server.fd = _serverFd;
	server.events = POLLIN;
	_poll_fds.push_back(server);

    std::cout << "Server started, waiting for clients..." << std::endl;

    while (true)
	{
        int pollCount = poll(this->_poll_fds.data(), _poll_fds.size(), TIMEOUT);
        if (pollCount < 0)
		{
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        }
		else if (pollCount == 0) // ffornes- Can remove this entire condition block....
		{
        	std::cout << "Poll timed out, no activity" << std::endl;
            continue;
        }
		for (size_t i = 0; i < _poll_fds.size(); i++) 
		{
			if (this->_poll_fds[i].revents & POLLIN)
			{
				if (this->_poll_fds[i].fd == this->_serverFd)
					newClient();
				else
					getClientData(i);
				/*
Clients like Irssi may automatically request capabilities (CAP LS, CAP REQ, etc.) right after the connection. This is part of the IRCv3 
extensions, where clients try to negotiate optional features (e.g., for encryption, SASL authentication, etc.).
    To handle this, you can:
        Block the capabilities exchange until the password is verified. This means the server should not respond to CAP LS or process any other 
		client commands until the correct password is provided.
        Send a PASS command response after successful authentication. Once the password is correct, your server can respond to CAP LS and allow 
		the handshake to continue.

To avoid processing commands like CAP LS or NICK prematurely, ensure that:
    No commands (like NICK, USER, CAP LS, etc.) are processed until the password is verified.
    Buffer the commands or reject them with a temporary error message (e.g., ERR_NOTREGISTERED with an appropriate message) if they are received 
	before the password is validated.
				*/
			}
		}
    }
}

void Server::newClient( void ) 
{
	struct sockaddr_storage	clientAddress;
	socklen_t	addressLen = sizeof(clientAddress);
	int	clientFd = accept(_serverFd, (struct sockaddr *)&clientAddress, &addressLen); // ffornes- other ircs set 2nd 3rd args as NULL ???
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
	struct pollfd	tmp;

	tmp.fd = clientFd;
	tmp.events = POLLIN;
	tmp.revents = 0;
	_poll_fds.push_back(tmp);
	this->_clients.insert(std::pair<int, Client>(clientFd, Client(clientFd)));
	this->_client_count++;
}

void	Server::getClientData( int i ) 
{
	char	buffer[BUFFER_SIZE] = {0};
	memset(buffer, 0, BUFFER_SIZE);
	int		bytes_received = read(this->_poll_fds[i].fd, buffer, BUFFER_SIZE - 1);

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
		std::vector<struct pollfd>::iterator it = _poll_fds.begin();
		std::advance(it, i);
		_poll_fds.erase(it);
	}
	else 
	{
		if (hasNULL(buffer, bytes_received))
			return ;
	
		buffer[bytes_received] = '\0';

		std::string message(buffer);

		parseData(message, this->_poll_fds[i].fd);
		sendData(buffer);
	}
}

/// apresas-: WIP
// ffornes-:	What's the point of this function? It barely does anything else besides calling prepareMessage
void Server::parseData( const std::string & raw_message, int client_fd )
{
	std::cout << "MESSAGE RECEIVED: " << raw_message;

	this->_current_client = &this->_clients[client_fd];
	t_message	message = prepareMessage(raw_message);
	message.sender_client_fd = client_fd;

	if (message.command.empty()) 
	{
		std::cerr << "Empty command received, message will be silently ignored" << std::endl;
		return ;
	}

//	std::cout << "Message received: ";
//	printTmessage(message);
	std::vector<t_message> replies = runCommand(message);
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

static void	stringToUpper( std::string & str, std::string src )
{
	for (std::string::iterator it = src.begin(); it != src.end(); it++)
		str += toupper(*it);
}

/*
apresas-: WIP, I will at some point make a map of function pointers with their names as keys to avoid the
if-else chain
*/
std::vector<t_message>	Server::runCommand( t_message & message ) 
{
	std::vector<t_message> replies;
	std::string	command;
	stringToUpper(command, message.command);

	if (command == "/PASS")
		return this->cmdPass(message);
	else if (command == "/NICK")
	{
		/*
			Expected response:

		*/
		return this->cmdNick(message);
	}
	else if (command == "/USER")
	{
		/*

		*/
		return this->cmdUser(message);
	}
	else if (command == "CAP" && !this->_current_client->isAuthorised())
	{
		// Must handle the CAP LS that irssi client sends when connecting...
		t_message	msg;
		msg.command = "CAP";
		msg.params[0] = "*";
		msg.params[1] = "LS";
		replies.push_back(msg); // Respond with CAP * LS
		// Advance the message until you find the next command NICK, adapt it with the correct params
		//	and call runCommand again, saving the reply in this replies.

		// Advance the message until you find the next command USER, adapt it with the correct params
		//	and call runCommand again, saving the reply in this replies.

		// Send reply number 900 asking for password...
		replies.push_back(createReply(900, "<nickname>"));
		return replies;

	}
	else if (!this->_current_client->isAuthorised())
	{
		// Maybe is too drastic to kick the client? 
		std::cout << "REMOVING CLIENT IN RUNCOMMAND FUNCTION" << std::endl;
		removeClient(message.sender_client_fd);
	}
	else if (command == "/MODE")
		return this->cmdMode(message);
	else if (command == "/JOIN")
		return this->cmdJoin(message);
	else if (command == "/QUIT")
		return this->cmdQuit(message);
	else
		replies.push_back(createReply(ERR_UNKNOWNCOMMAND, ERR_UNKNOWNCOMMAND_STR, message.command));
	return replies;
}

bool	Server::hasNULL( const char * buffer, int bytes_received ) const
{
	for (int i = 0; i < bytes_received; i++)
		if (buffer[i] == '\0')
			return true;
	return false;
}

bool	Server::hasCRLF( const std::string str ) const
{
	if (str.size() > 1)
		return str[str.size() - 2] == '\r' && str[str.size() - 1] == '\n';
	return false;
}

void	Server::printTmessage( t_message message ) const 
{
	std::cout << "Prefix [" << message.prefix << "] ";
	std::cout << "Command [" << message.command << "] ";
	std::cout << "Params ";
	for (size_t i = 0; i < message.params.size(); i++)
		std::cout << "[" << message.params[i] << "] ";
	std::cout << std::endl;
}
