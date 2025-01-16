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
	this->_clients.insert(std::pair<int, Client>(clientFd, client));
	this->_client_count++;
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

		Client	& current_client = this->_clients[this->_poll_fds[i].fd];
		std::string	message;
		// Check if the clients buffer contains something...
		if (current_client.getBuffer().size() > 0)
		{
			message = current_client.getBuffer();
			current_client.clearBuffer();
		}
		message += buffer;

		std::vector<std::string> messages = splitMessage(message);

		if (messages.size() > 0)
			std::cout << "After message split, size: " << messages[0].size() << std::endl;
		else // I have to save content in the clients buffer then
			if (current_client.fillBuffer(buffer)) // Then buffer is filled to the brim and needs to be sent
				messages.push_back(current_client.getBuffer());

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
		final_reply += " " + *it;
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

	this->_current_client = &this->_clients[client_fd];
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

	std::cout << "runCommand function called..." << std::endl;

	if (this->_current_client->isTerminate())
	{
		// Close the connection here, send the ERROR message and close the connection with the client
	}
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
			After the USER command, we should check if the client is authorised.
			If it's NOT, we should set its terminate flag to true
		*/
		replies = this->cmdUser(message);
		if (this->_current_client->isAuthorised() == false)
			this->_current_client->setTerminate(true);
		return replies;
	}
	else if (command == "CAP" && !this->_current_client->isAuthorised())
	{
		// Our server will have no extensions, so we will silently ignore the opening CAP command
		return replies;
	}
	else if (command == "JOIN" && !this->_current_client->isRegistered())
	{
		// For the message JOIN : that irssi sends right after CAP
		// From what I've seen, here is where most servers lookup the hostname of the client and notify them with a NOTICE
		// TODO: We can make this use the cmdNotice function when we have it
		t_message notice_lookup;
		notice_lookup.prefix = ":" + this->getName();
		std::cout << "notice_lookup.prefix: \"" << notice_lookup.prefix << "\"" << std::endl;
		notice_lookup.command = "NOTICE";
		notice_lookup.target_client_fds.insert(this->_current_client->getSocket());
		notice_lookup.params.push_back("AUTH"); // Or maybe "*" ?
		notice_lookup.params.push_back(":*** Looking up your hostname");
		replies.push_back(notice_lookup);

		t_message notice_results;
		notice_results.prefix = ":" + this->getName();
		notice_results.command = "NOTICE";
		notice_results.target_client_fds.insert(this->_current_client->getSocket());
		notice_results.params.push_back("AUTH"); // Or maybe "*" ?
		notice_results.params.push_back(this->_current_client->hostnameLookup());
		replies.push_back(notice_results);

		replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
	}
	// else if (!this->_current_client->isAuthorised())
	// {
	// 	// Maybe is too drastic to kick the client?
	// 	std::cout << "REMOVING CLIENT IN RUNCOMMAND FUNCTION" << std::endl;
	// 	removeClient(message.sender_client_fd);
	// }
	else if (command == "MODE")
		return this->cmdMode(message);
	else if (command == "JOIN")
		return this->cmdJoin(message);
	else if (command == "QUIT")
		return this->cmdQuit(message);
	else if (command == "ERROR")
		return replies; // Silently ignore the ERROR command from a client
	else if (command == "NOTICE")
		return this->cmdNotice(message);
	else if (command == "VERSION")
		return this->cmdVersion(message);
	else if (command == "TIME")
		return this->cmdTime(message);
	else
		replies.push_back(createReply(ERR_UNKNOWNCOMMAND, ERR_UNKNOWNCOMMAND_STR, message.command));
	return replies;
}

