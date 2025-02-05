#include <cerrno>
#include "Server.hpp"

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
		if (pollCount == 0) {}
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

void Server::removeTerminatedClients( void )
{
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
}

void Server::removeClient( int fd )
{
	Client * client = this->findClient(fd);

	if (!client)
	{
		return ;
	}
	// Remove client's name from _taken_nicknames ... std::vector<std::string>()
	std::vector<std::string>::iterator taken_nickname_it = std::find(_taken_nicknames.begin(), _taken_nicknames.end(), client->getNickname());
	if (taken_nickname_it != _taken_nicknames.end())
		_taken_nicknames.erase(taken_nickname_it);
	// Remove from _clients_fd_map ... std::map<std::string, int>()
	if (_clients_fd_map.find(client->getNickname()) != _clients_fd_map.end()) // Check if it exists
		_clients_fd_map.erase(client->getNickname());
	// Remove from _clients ... std::map<int, Client>()
	if (_clients.find(fd) != _clients.end()) // Check if it exists
		_clients.erase(fd);
	if (close(fd) == -1) // error handling...
		closeFailureLog("_poll_fds", fd, this->_serverFd);
	// Remove from _poll_fds .	std::vector<struct pollfd>()
	for (std::vector<struct pollfd>::iterator it = _poll_fds.begin(); it != _poll_fds.end(); ++it)
	{
		if ((*it).fd == fd )
		{
			_poll_fds.erase(it);
			break ;
		}
	}
	this->_client_count -= 1;
}

