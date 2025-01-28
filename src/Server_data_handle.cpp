#include "Server.hpp"

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
		{
			t_message	message = parseData(*it, this->_poll_fds[i].fd);
			if (message.command.empty())
			{
				std::cerr << "Empty command received, message will be silently ignored" << std::endl;
				continue;
			}

			std::vector<t_message> replies = runCommand(message);
			for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
			{
				sendReply(*it);
			}
			this->_current_client->setLastActivity();
		}
	}
}

/// PROVISIONAL
std::vector<std::string> Server::splitMessage( std::string & message )
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

/*
apresas-: WIP, I will at some point make a map of function pointers with their names as keys to avoid the
if-else chain
*/
std::vector<t_message>	Server::runCommand( t_message & message ) 
{
	std::vector<t_message> replies;
	std::string	command = stringToUpper(message.command);

	if (command == "CAP")
		return replies;
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
	else if (command == "PART")
		replies = cmdPart(message);
	else if (command == "MOTD")
		replies = cmdMotd(message);
	else if (command == "NAMES")
		replies = cmdNames(message);
	else if (command == "INFO")
		replies = cmdInfo(message);
	else if (command == "LIST")
		replies = cmdList(message);
	else if (command == "WHO")
		replies = cmdWho(message);
	else if (this->_current_client->isRegistered() == true)
		replies.push_back(createReply(ERR_UNKNOWNCOMMAND, ERR_UNKNOWNCOMMAND_STR, message.command));
	return replies;
}

