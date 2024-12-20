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

// TODO finish this shit
/*
	Redo the t_message struct to be able to handle replies or create a new struct to handle replies since
	replies don't containt commands + if we need to send a reply to a channel we are unable to do so using
	t_message since it only stores fds and it could be fixed storing a container of fds with all the fds
	that we want to reach but it's way more fucking easier to have a Channel pointer or something like that
	that has it's clients and from there we can access them and send the message to all of them
*/
static void	sendReplies( t_message reply )
{
	size_t	message_size = 0;
	//const char *	cmd = reply.command.c_str();

	message_size += reply.command.size() + 1;
	for (std::vector<std::string>::iterator it = reply.params.begin(); it != reply.params.end(); ++it)
		message_size += (*it).size() + 1;
	
	char *	message = new char[message_size];

	strcpy(message, reply.command.c_str());
	message[reply.command.size()] = ' ';
	for (std::vector<std::string>::iterator it = reply.params.begin(); it != reply.params.end(); ++it)
	{
		//............. fill message...........
	}
	message[message_size] == '\0';

//	if ( target is not a channel ... we need to verify this )
	send(reply.target_client_fd, message, message_size, 0);
//	else ( target is a channel )
//	sendToChannel(message, message_size);
}

/// apresas-: WIP
// ffornes- this does too much I swear I'm gonna change it some day
void Server::parseData( const std::string & raw_message, int client_fd )
{
//	std::cout << "MESSAGE RECEIVED: " << raw_message;

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
	for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
	{
		printTmessage(*it);
		//TODO send replies...
		sendReplies(*it);
		//send((*it).target_client_fd, 
	}
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

static std::string	stringToUpper( std::string src )
{
	std::string str;
	for (std::string::iterator it = src.begin(); it != src.end(); it++)
		str += toupper(*it);
	return str;
}

/*
apresas-: WIP, I will at some point make a map of function pointers with their names as keys to avoid the
if-else chain
*/
std::vector<t_message>	Server::runCommand( t_message & message ) 
{
	std::vector<t_message> replies;
	std::string	command = stringToUpper(message.command);

	std::cout << "PRINTING MESSAGE" << std::endl;
	printTmessage(message);
	std::cout << "END PRINTING MESSAGE" << std::endl;
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
		std::vector<std::string>::iterator it = message.params.begin();
		t_message	msg; // Remember to add info about sender and target
		msg.command = message.command;
		msg.sender_client_fd = this->_serverFd;
		msg.target_client_fd = message.sender_client_fd;
		while (stringToUpper(*it) != "NICK" && it != message.params.end())
		{
			msg.params.push_back(*it);
			std::advance(it, 1);
		}
		replies.push_back(msg); // Respond with CAP * LS
		// Advance the message until you find the next command NICK, adapt it with the correct params
		//	and call runCommand again, saving the reply in this replies.
		if (it != message.params.end())
		{
			t_message	msg_nick; // Remember to add info about sender and target
			msg_nick.command = *it;
			msg_nick.sender_client_fd = this->_serverFd;
			msg_nick.target_client_fd = message.sender_client_fd;
			std::advance(it, 1);
			while (stringToUpper(*it) != "USER" && it != message.params.end())
			{
				msg_nick.params.push_back(*it);
				std::advance(it, 1);
			}
			replies.push_back(msg_nick);
		}
		// Advance the message until you find the next command USER, adapt it with the correct params
		//	and call runCommand again, saving the reply in this replies.
		if (it != message.params.end())
		{
			t_message	msg_user; // Remember to add info about sender and target
			msg_user.command = *it;
			msg_user.sender_client_fd = this->_serverFd;
			msg_user.target_client_fd = message.sender_client_fd;
			std::advance(it, 1);
			while (it != message.params.end())
			{
				msg_user.params.push_back(*it);
				std::advance(it, 1);
			}
			replies.push_back(msg_user);
		}
		// Send reply number 900 asking for password...
		// TODO createReply doesnt fill the target............
		replies.push_back(createReply(900, this->_current_client->getNickname()));
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
	std::cout << "Sender: " << message.sender_client_fd << std::endl;
	std::cout << "Target: " << message.target_client_fd << std::endl;
	std::cout << std::endl;
}
