#include "Server.hpp"

// Server::Server( void )
// {
// 	this->_fd = -1;
// 	this->_status = false;
// 	this->_name = "";
// 	this->_port = 0;
// 	this->_password = "";
// }

Server::Server( t_port port, const std::string & password ) : _port(port), _password(password)
{
	this->_status = false;
	try
	{
		this->initSocket();
		this->setSocketOptions();
		this->bindSocket();
		this->initListen();
		// this->acceptConnection();
		// this->receiveData();
		// this->sendData( "Hello from server!" );
		// this->closeConnection();
	}
	catch ( const std::exception & e )
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}
}

Server::~Server( void )
{
	close(this->_fd); // ? Maybe should be careful to check it has not been already closed
}

int	Server::getFd( void ) const
{
	return this->_fd;
}

void	Server::initSocket( void )
{
	this->_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (this->_fd == -1)
		throw SocketCreationException();
}

void	Server::setSocketOptions( void )
{
	int	opt = 1;
	if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) != 0)
	{
		close(this->_fd);
		throw SetsockoptException();
	}
}

void	Server::bindSocket( void )
{
	struct sockaddr_in	address;
	std::memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY; // Bind to all available interfaces
	address.sin_port = htons(this->_port);

	if (bind(this->_fd, (struct sockaddr*)&address, sizeof(address)) < 0)
	{
		close(this->_fd);
		throw BindException();
	}
}

void	Server::initListen( void )
{
	if (listen(this->_fd, 3) < 0)
	{
		close(this->_fd);
		throw ListenFailedException();
	}
}

void	Server::acceptConnection( void )
{
	struct sockaddr_in	client_address;
	socklen_t	client_address_len = sizeof(client_address);
	this->_client_fd = accept(this->_fd, (struct sockaddr*)&client_address, &client_address_len);
	if (this->_client_fd < 0)
	{
		close(this->_fd);
		throw AcceptFailedException();
	}

	std::cout << "Connection established with client: "
			  << inet_ntoa(client_address.sin_addr) << ":"
			  << ntohs(client_address.sin_port) << std::endl;
}

void	Server::receiveData( void )
{
	char	buffer[BUFFER_SIZE] = {0};
	int		bytes_received = recv(this->_client_fd, buffer, sizeof(buffer), 0);
	if (bytes_received < 0)
	{
		close(this->_client_fd);
		close(this->_fd);
		throw RecvFailedException();
	}

	std::cout << "Received: " << buffer << std::endl;
}

void	Server::sendData( const std::string & data )
{
	// send(this->_client_fd, data.c_str(), data.length(), 0);
	// I think this is equivalent to this:
	write(this->_client_fd, data.c_str(), data.length());
	// write doesn't accept flags, but in this case we are not using any
}

void	Server::closeConnection( void )
{
	close(this->_client_fd);
}

void	Server::start( void )
{
	// if (this->_isServerIsReady() == true)
		this->_status = true;
	// Server loop goes here ?
	struct pollfd fds[MAX_CLIENTS];
	for (int i = 0; i < MAX_CLIENTS; i++)
	{
		fds[i].fd = -1;
	}

	fds[0].fd = this->_fd;
	fds[0].events = POLLIN;

	int client_count = 1;

	while (true)
	{
		int	poll_count = poll(fds, client_count, -1);
		if (poll_count < 0)
		{
			std::cerr << "Poll failed." << std::endl; // Make an exception later
		}

		for (int i = 0; i < client_count; i++)
		{
			if (fds[i].events & POLLIN)
			{
				if (fds[i].fd == this->_fd)
				{
					// New connection
					struct sockaddr_in	client_address;
					socklen_t	client_address_len = sizeof(client_address);
					int client_fd = accept(this->_fd, (struct sockaddr*)&client_address, &client_address_len);
					if (client_fd < 0)
					{
						AcceptFailedException();
						continue ;
					}

					std::cout << "New connection from "
							  << inet_ntoa(client_address.sin_addr) << ":"
							  << ntohs(client_address.sin_port) << std::endl;
					
					if (client_count > MAX_CLIENTS)
					{
						std::cerr << "Max clients reached. Closing connection." << std::endl;
						close(client_fd);
						continue ;
					}
					fds[client_count].fd = client_fd;
					fds[client_count].events = POLLIN;
					client_count++;
				}
				else
				{
					// Existing connection
					char buffer[1024] = {0};
                    int bytes_received = recv(fds[i].fd, buffer, sizeof(buffer), 0);
                    if (bytes_received <= 0) {
                        // Client disconnected or error
                        std::cout << "Client disconnected: FD " << fds[i].fd << std::endl;
                        close(fds[i].fd);
                        fds[i].fd = -1;
                    }
					else
					{
                        std::cout << "Received: " << buffer << " from FD " << fds[i].fd << std::endl;
                        const char* response = "Hello from server!";
                        send(fds[i].fd, response, strlen(response), 0);
					}
				}
			}
		}

		// Compact the poll list if needed
        int new_client_count = 1; // Start with the server socket
        for (int i = 1; i < client_count; ++i) {
            if (fds[i].fd != -1) {
                fds[new_client_count++] = fds[i];
            }
        }
        client_count = new_client_count;
	}
}

// bool	Server::_isServerReady( void )
// {
// 	if (this->_fd == -1)
// 		return false;
	
// }

void	Server::stop( void )
{
	this->_status = false;
}