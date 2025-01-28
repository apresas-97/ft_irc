#include "Server.hpp"

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
	sendReply(hostname_lookup_notice);
	t_message	hostname_lookup_results_notice = this->createNotice(&client, client.hostnameLookup());
	sendReply(hostname_lookup_results_notice);
}
