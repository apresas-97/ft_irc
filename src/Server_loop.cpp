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
//	int		bytes_received = recv(this->_poll_fds[i].fd, buffer, sizeof(buffer) - 1, 0);

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
		// Check if there's any NULL characters in the buffer... if there are, the message should be silently ignored
		if (hasNULL(buffer, bytes_received))
			return ; // ffornes- maybe not a simple return here?
	
		buffer[bytes_received] = '\0'; // Maybe this overwrites the last character received from recv, but I'm not sure
									   // ffornes- it's necessary in order to check for CRLF, no null == no end??

		// ffornes- We were supposed to look for CRLF but every test I did I wasn't able to find them in any message

//		Client * client = findClient(_poll_fds[i].fd); // ffornes- maybe do it without a pointer?

		std::string message(buffer);

//		client->addToBuffer(buffer); // Check if returned true
//		std::string	client_buffer = client->getBuffer();
//		std::string message(client_buffer, client_buffer.size() - 1); // apresas-: Maybe just message(buffer); ?
//		std::string response;

		parseData(message, this->_poll_fds[i].fd);
		sendData(buffer);
//		client->cleanBuffer();

//		delete client; // SHould we handle the client in a different way? If I delete it explodes LMAO

		/* apresas-:
			TODO:
			
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
			
		*/
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
	std::vector<t_message> replies = runCommand(message, client_fd);

	// apresas-: At this point, the replies should be ready to be processed back to raw data and sent back to the client
	// For now this will only work for commands that will be returned to the sender
	// Stil need to implement putting the fd of the target of the message in the t_message struct
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
std::vector<t_message>	Server::runCommand( t_message & message, int fd ) 
{
	std::vector<t_message> replies;
	std::string	command;
	stringToUpper(command, message.command);

	std::cout << "COMMAND in runCommand: " << command << std::endl;
	if (command == "/PASS")
		return this->cmdPass(message);
	else if (command == "/NICK")
		return this->cmdNick(message);
	else if (command == "/USER")
		return this->cmdUser(message);
	else if (command == "/MODE")
		return this->cmdMode(message);
	else if (command == "/JOIN")
		return this->cmdJoin(message);
	else if (command == "/QUIT")
		return this->cmdQuit(message, fd);
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
	std::cout << "Prefix [" << message.prefix << "]" << std::endl;
	std::cout << "Command [" << message.command << "]" << std::endl;
	std::cout << "Params ";
	for (size_t i = 0; i < message.params.size(); i++)
		std::cout << "[" << message.params[i] << "] ";
	std::cout << std::endl;
}
