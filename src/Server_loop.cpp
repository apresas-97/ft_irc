#include <cstring>
#include <cerrno>
#include "ft_irc.hpp"
#include "Server.hpp"

#define PORT 8080 // Port number to bind

// FOR NOW HERE
#define CLIENT_TIMEOUT_SECONDS 600 // Seconds before a client should be sent a PING message to check if it's still alive
#define CLIENT_PING_TIMEOUT_SECONDS 30 // Seconds before a connection is considered dead after no PONG reply is received
#define CLIENT_REGISTRATION_TIMEOUT_SECONDS 10 // Seconds before a client is considered not registered and disconnected
static void	sendReplies( t_message reply ); //

// FOR NOW HERE
void Server::checkInactivity( void )
{
	// std::cout << "CHECK INACTIVITY FUNCTION" << std::endl;
	// std::cout << "client count = " << this->_client_count << std::endl;
	if (this->_client_count == 0)
		return ;
	// std::cout << "TIMEOUT loop start" << std::endl;
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		Client & client = it->second;
		if (client.isTerminate())
			continue;
		if (client.isRegistered())
		{
			if (!client.isExpectedPong() && client.getLastActivity() + CLIENT_TIMEOUT_SECONDS < std::time(NULL))
			{
				// std::cout << "CLIENT TIMEOUT DETECTED" << std::endl;
				// std::cout << "Time now = " << std::time(NULL) << std::endl;
				// std::cout << "client last activity = " << client.getLastActivity() << std::endl;
				t_message ping_message;
				ping_message.prefix = ":" + this->getName(); // I've seen servers not sending prefix for PING messages
				ping_message.command = "PING";
				ping_message.params.push_back(":" + this->getName());
				ping_message.target_client_fds.insert(it->first);
				sendReplies(ping_message);
				client.setExpectedPong(true);
				client.setPongTimer();
			}
			if (client.isExpectedPong() && client.getPongTimer() + CLIENT_PING_TIMEOUT_SECONDS < std::time(NULL))
			{
				// std::cout << "CLIENT PONG TIMEOUT DETECTED" << std::endl;
				// std::cout << "Time now = " << std::time(NULL) << std::endl;
				// std::cout << "client pong timer = " << client.getPongTimer() << std::endl;
				this->_current_client = &client;
				t_message quit_message;
				quit_message.prefix = ":" + client.getUserPrefix();
				quit_message.command = "QUIT";
				quit_message.params.push_back("Ping timeout");
				std::vector<t_message> replies = this->cmdQuit(quit_message);
				for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
				{
					printTmessage(*it); // DEBUG
					sendReplies(*it);
				}
			}
		}
		else if (client.getLastActivity() + CLIENT_REGISTRATION_TIMEOUT_SECONDS < std::time(NULL))
		{
			this->_current_client = &client;
			client.setHostname("0.0.0.0");
			t_message quit_message;
			quit_message.prefix = ":" + client.getUserPrefix();
			quit_message.command = "QUIT";
			quit_message.params.push_back("Ping timeout");
			std::vector<t_message> replies = this->cmdQuit(quit_message);
			for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
			{
				printTmessage(*it); // DEBUG
				sendReplies(*it);
			}
		}
	}
	// std::cout << "CHECK INACTIVITY FUNCTION END" << std::endl;
}

void Server::removeTerminatedClients( void )
{
	std::cout << "REMOVE TERMINATED CLIENTS FUNCTION" << std::endl;
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		if (it->second.isTerminate())
		{
			removeClient(it->first);
			if (this->_clients.size() == 0)
				break;
			it = this->_clients.begin();
		}
	}
	std::cout << "REMOVE TERMINATED CLIENTS FUNCTION END" << std::endl;
}

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
		if (pollCount == 0)
		{
        	std::cout << "Poll timed out, no activity" << std::endl; // Remove later...
        }
		else
		{
			for (size_t i = 0; i < _poll_fds.size(); i++) 
			{
				if (this->_poll_fds[i].revents & POLLIN)
				{
					if (this->_poll_fds[i].fd == this->_serverFd)
						newClient();
					else
					{
						this->_current_client = this->findClient(this->_poll_fds[i].fd);
						getClientData(i);
					}
				}
			}
		}
		this->checkInactivity();
		this->removeTerminatedClients();
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
			cleanClose(false);
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
	setupClientHostname(client);
	client.hostnameLookup();
	this->_clients.insert(std::pair<int, Client>(clientFd, client));
	this->_client_count++;
}

void Server::setupClientHostname( Client & client )
{
	t_message	hostname_lookup_notice = this->createNotice(&client, "*** Looking up your hostname...");
	sendReplies(hostname_lookup_notice);
	t_message	hostname_lookup_results_notice = this->createNotice(&client, client.hostnameLookup());
	sendReplies(hostname_lookup_results_notice);
}

/// PROVISIONAL
std::vector<std::string> splitMessage( std::string & message )
{
	std::vector<std::string> messages;
	std::string delimiter = "\r\n";

	size_t pos = 0;
	std::string token;
	while ((pos = message.find(delimiter)) != std::string::npos)
	{
		token = message.substr(0, pos);
		messages.push_back(token);
		message.erase(0, pos + delimiter.length());
	}
	return messages;
}
///

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

		std::string	message;
		// Check if the clients buffer contains something...
		if (this->_current_client->getBuffer().size() > 0)
		{
			message = this->_current_client->getBuffer();
			this->_current_client->clearBuffer();
		}
		message += buffer;

		std::vector<std::string> messages = splitMessage(message);

		// Client buffer handling...
		if (messages.size() == 0 || message.size() > 0) // If we didn't find CRLF in the message OR if there is something after the CRLF
		{
			if (this->_current_client->fillBuffer(message))
			{
				messages.push_back(this->_current_client->getBuffer());
				this->_current_client->clearBuffer();
			}
		}
		
		// Iterate over the split messages and parse them
		for (std::vector<std::string>::iterator it = messages.begin(); it != messages.end(); ++it)
			parseData(*it, this->_poll_fds[i].fd);
	}
}

static std::string	formatReply( t_message reply )
{
	std::string	final_reply;

	if (reply.prefix.size() > 0)
		final_reply += reply.prefix + " ";
	final_reply += reply.command;
	for (std::vector<std::string>::iterator it = reply.params.begin(); it != reply.params.end(); ++it)
	{
		if (it + 1 == reply.params.end())
		{
			std::string last_param = *it;
			if (last_param.find(' ') != std::string::npos)
				final_reply += " :" + last_param;
			else
				final_reply += " " + last_param;
		}
		else
			final_reply += " " + *it;
	}
	final_reply += "\r\n";
	return final_reply;
}

static void	sendReplies( t_message reply )
{
	// std::cout << "Sending replies..." << std::endl;
	std::string	output = formatReply(reply);

	for (std::set<int>::iterator it = reply.target_client_fds.begin(); it != reply.target_client_fds.end(); ++it)
	{
		std::cout << "Reply target fd: " << *it << std::endl; // DEBUG
		send(*it, output.c_str(), output.size(), 0);
	}
}

/// apresas-: WIP
void Server::parseData( const std::string & raw_message, int client_fd )
{
	std::cout << "parseData function called..." << std::endl; // DEBUG
	std::cout << "MESSAGE RECEIVED: \"" << raw_message << "\"" << std::endl; //  DEBUG

	t_message	message = prepareMessage(raw_message);
	message.sender_client_fd = client_fd;

	if (message.command.empty())
	{
		std::cerr << "Empty command received, message will be silently ignored" << std::endl;
		return ;
	}

	printTmessage(message); // DEBUG

	std::vector<t_message> replies = runCommand(message);
	for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
	{
		printTmessage(*it); // DEBUG
		sendReplies(*it);
	}
	this->_current_client->setLastActivity();
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
			word.erase(0, 1); // Remove the ':' character
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

	std::cout << "runCommand function called..." << std::endl;
	if (command == "CAP" && !this->_current_client->isAuthorised())
		replies = this->cmdCap(message);
	else if (command == "PASS")
		replies = cmdPass(message);
	else if (command == "NICK")
		replies = cmdNick(message);
	else if (command == "USER")
		replies = this->cmdUser(message);
	else if (command == "QUIT")
		replies = cmdQuit(message);
	else if (command == "VERSION")
		replies = cmdVersion(message);
	else if (command == "ERROR")
		return replies; // Silently ignore the ERROR command from a client
	else if (command == "MODE")
		replies = cmdMode(message);
	else if (command == "JOIN")
		replies = cmdJoin(message);
	else if (command == "NOTICE")
		replies = cmdNotice(message);
	else if (command == "TIME")
		replies = cmdTime(message);
	else if (command == "PRIVMSG")
		replies = cmdPrivMsg(message);
	else if (command == "INVITE")
		replies = cmdInvite(message);
	else if (command == "TOPIC")
		replies = cmdTopic(message);
	else if (command == "PING")
		replies = cmdPing(message);
	else if (command == "PONG")
		replies = cmdPong(message);
	else if (command == "KICK")
		replies = cmdKick(message);
	else if (command == "MOTD")
		replies = cmdMotd(message);
	else if (this->_current_client->isRegistered() == true)
		replies.push_back(createReply(ERR_UNKNOWNCOMMAND, ERR_UNKNOWNCOMMAND_STR, message.command));
	return replies;
}
