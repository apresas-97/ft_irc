#include <cstring>
#include <cerrno>
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
		else if (pollCount == 0)
		{
        	std::cout << "Poll timed out, no activity" << std::endl; // Remove later...
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
	Client client(clientFd);
	client.setSockaddr((struct sockaddr *)&clientAddress);
	this->_clients.insert(std::pair<int, Client>(clientFd, client));
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
		removeClient(this->_poll_fds[i].fd);
	}
	else 
	{
		if (hasNULL(buffer, bytes_received))
			return ;
	
		buffer[bytes_received] = '\0';

		std::string message(buffer);

		parseData(message, this->_poll_fds[i].fd);
		//	sendData(buffer); // ffornes- we do not send data here we send it with the replies
	}
}

static std::string	formatReply( t_message reply )
{
	std::string	final_reply;

	if (reply.prefix.size() > 0)
		final_reply += reply.prefix + " ";
	final_reply += reply.command;
	for (std::vector<std::string>::iterator it = reply.params.begin(); it != reply.params.end(); ++it)
		final_reply += " " + *it;
	final_reply += "\r\n";
	return final_reply;
}

static void	sendReplies( t_message reply )
{
	// std::cout << "Sending replies..." << std::endl;
	std::string	output = formatReply(reply);

	if (reply.target_channels.size() > 0)
	{
		std::cout << "Target is 1 or more channels..." << std::endl;
		// sendToChannel(reply);
	}
	else
	{
		std::cout << "Reply target fd: " << reply.target_client_fd << std::endl;
	//	std::cout << "Attempting to send message to irssi......." << std::endl;
	//	output = "NOTICE AUTH :*** Looking up your hostname\r\n"; // ffornes- THIS IS ACCEPTED BY IRSSI which means this kindof the format expected...
		send(reply.target_client_fd, output.c_str(), output.size(), 0);
	}
}

/// apresas-: WIP
// ffornes- this does too much I swear I'm gonna change it some day
void Server::parseData( const std::string & raw_message, int client_fd )
{
	// std::cout << "parseData function called..." << std::endl;
	// std::cout << "MESSAGE RECEIVED: \"" << raw_message << "\"" << std::endl;

	this->_current_client = &this->_clients[client_fd];
	t_message	message = prepareMessage(raw_message);
	message.sender_client_fd = client_fd;
	message.target_client_fd = -1;

	if (message.command.empty())
	{
		std::cerr << "Empty command received, message will be silently ignored" << std::endl;
		return ;
	}

	printTmessage(message);

	std::vector<t_message> replies = runCommand(message); // target must be set in run command...
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
t_message	Server::prepareMessage( std::string raw_message ) // ffornes- Where do we set the target??? 
{
	// std::cout << "prepareMessage function called..." << std::endl;
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

/*
apresas-: WIP, I will at some point make a map of function pointers with their names as keys to avoid the
if-else chain
*/
std::vector<t_message>	Server::runCommand( t_message & message ) 
{
	std::vector<t_message> replies;
	std::string	command = stringToUpper(message.command);

	printTmessage(message);
	if (command == "PASS")
		return this->cmdPass(message);
	else if (command == "NICK")
	{
		/*
			Expected response:

		*/
		return this->cmdNick(message);
	}
	else if (command == "USER")
	{
		/*

		*/
		return this->cmdUser(message);
	}
	else if (command == "CAP" && !this->_current_client->isAuthorised()) // ffornes- :: TODO............
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

//		replies.push_back(createReply(900, this->_current_client->getNickname()));

		t_message	passReply = createReply(900, this->_current_client->getNickname());
		passReply.sender_client_fd = this->_serverFd;
		passReply.target_client_fd = message.sender_client_fd;
		replies.push_back(passReply);

		return replies;
	}
	else if (!this->_current_client->isAuthorised())
	{
		// Maybe is too drastic to kick the client?
		std::cout << "REMOVING CLIENT IN RUNCOMMAND FUNCTION" << std::endl;
		removeClient(message.sender_client_fd);
	}
	else if (command == "MODE")
		return this->cmdMode(message);
	else if (command == "JOIN")
		return this->cmdJoin(message);
	else if (command == "QUIT")
		return this->cmdQuit(message);
	else
		replies.push_back(createReply(ERR_UNKNOWNCOMMAND, ERR_UNKNOWNCOMMAND_STR, message.command));
	return replies;
}

