#include <iostream>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <poll.h>

#include <arpa/inet.h>

#define	MAX_CLIENTS	1024
#define	TIMEOUT		5000
#define	BUFFER_SIZE 1024

void	sendData( struct pollfd	*pollFds, char *buffer );

int main( int argc, char *argv[] ) {
	if ( argc != 3 ) {
		std::cout << "Usage: ./server <port> password" << std::endl;
		return -1;
	}

	/*
		Create server socket 
			int socket( int domain, int type, int protocol );

				domain:		AF_INET		= IPv4 Internet protocols
							AF_INET6	= IPv6 Internet protocols
				type:		SOCK_STREAM	= Sequenced reliable two-way connection-based byte streams
				protocol:	SOCK_NONBLOCK	= Set the O_NOBLOCK file status flag on the open file
											  descriptor.
	*/
	int	serverFd;
	serverFd = socket( AF_INET, SOCK_STREAM, 0 );
	if ( serverFd < 0 ) {
		std::cerr << "Server socket creation failed." << std::endl;
		return -1;
	}

	/*
		Bind the socket to a specific IP address and port
			int bind ( int sockfd, const struct sockaddr*, socklen_t addrlen );
				sockfd:		The file descriptor that references the socket
				sockaddr*:	The specified address that will be assigned to the socket
					struct sockaddr {
						sa_familty_t	sa_family;
						char			sa_data[14];
					}
				addrlen:	The specified size in bytes of the address structure pointed to by addr
		
		PORT:	Should use ports from the range 6660 - 6669 those are the ports most IRC servers use
				Port 6697 or 7000 is used if you offer encrypted communication between clients and server
	*/
	int	port;
	port = atoi( argv[1] );
	if ( port < 0 || port > 65535 ) {
		std::cerr << "Invalid port request, please enter a new port in the range 0-65535." << std::endl;
		close( serverFd );
		return -1;
	}

	struct	sockaddr_in	serverAddress;
	memset( &serverAddress, 0, sizeof(serverAddress) );
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);

	if ( bind( serverFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0 ) {
		std::cerr << "Server socket binding failed." << std::endl;
		close( serverFd );
		return -1;
	}

	/*
		Listen for connections
			int listen( int sockfd, int backlog );

			sockfd:		The file descriptor referencing the socket that's listening
			backlog:	Defines the maximum length to which the queue of pending connections may grow
						If a request arrives when the queue is full, the client may receive an error

	*/
	if ( listen(serverFd, MAX_CLIENTS) < 0 ) {
		std::cerr << "Listen failed." << std::endl;
		close( serverFd );
		return -1;
	}
	std::cout << "Server " << serverFd << " is listening on port " << port << std::endl;

	//	Poll file descriptors initialization
	struct pollfd	pollFds[ MAX_CLIENTS + 1 ];
	pollFds[0].fd = serverFd;
	pollFds[0].events = POLLIN;
	for ( size_t i = 1; i < MAX_CLIENTS; i++ ) {
		pollFds[i].fd = -1;
		pollFds[i].events = POLLIN;
	}

	std::cout << "Server started, waiting for clients. . ." << std::endl;

	/*
		Server loop
			int poll( struct pollfd, nfds_t, int timeout );
				pollfd:		All the file descriptors referencing the sockets we want to check for. It must be the server
							and the clients.	
				nfds_t:		Specifies the amount of connections it has to look for, clients + 1 since 1 is the server.
							It's necessary to specify clients instead of MAX_CLIENTS because if you check with MAX_CLIENTS
							it returns -1. clients represent the actual amount of clients connected to the server.	
				timeout:	Specifies the number of milliseconds that poll should block waiting for a fd to becomre ready

			Returns the number of elements in pollFds whose revents fields have been set to a non-zero value aka even or error
	
			this unsigned int clients causes issues since if you decrease it when a client is disconnected, it may be possible
			that poll doesn't check for other clients. Example:
				[Server] [client1] [client2]	here everything works fine, clients = 2, poll gets all the connections properly
			However if we disconnect client1...
				[Server] [       ] [client2]	here we are unable to receive data from client2 since it's in the second position
			of the pollFds array, this should be handled in some way. 
	*/
	unsigned int	clients = 0;
	while ( 42 ) {
		int	pollCount = poll( pollFds, clients + 1, TIMEOUT );
		if ( pollCount < 0 ) {
			std::cerr << "Poll error, errno: " << strerror(errno) << std::endl;
			break ;
		} else if ( pollCount == 0 ) {
			std::cout << "Poll timed out, no activity" << std::endl;
			continue;
		}

		/*
			Check server socket for new client connections
				int accept( int sockfd, struct sockaddr*, socklen_t );
					sockfd:		The file descriptor that references the server socket
					sockaddr*:	The address of the client I'm trying to accept
					socklen_t:	The size in bytes of the struct sockaddr. It must be initialized by the caller

			On success returns a file descriptor for the accepted socket.
			On error returns -1 and errno is set, addrlen is left unchanged.
		*/
		if ( pollFds[0].revents & POLLIN ) {
			int	clientFd;
			struct	sockaddr_in	clientAddress;
			socklen_t	addressLen = sizeof( clientAddress );

			clientFd = accept( serverFd, (struct sockaddr*)&clientAddress, &addressLen );
			if ( clientFd < 0 ) {	// Accept failed, jumping to the next iteration of the server loop
				std::cerr << "Accept new client failed" << std::endl;
				continue;
			}

			bool	clientAdded = false;
			for ( size_t i = 1; i < MAX_CLIENTS; i++ {
				if ( pollFds[i].fd == -1 ) {
					pollFds[i].fd = clientFd;
					std::cout << "New client connection: " << clientFd << std::endl;
					clientAdded = true;
					if ( i > clients )
						clients++;
					break ;
				
			}
			if ( !clientAdded ) {	// Unable to add new client, max connections reached
				std::cerr << "Unable to add new client, max number of connections reached" << std::endl;
				close( clientFd );
			}
		}

		/*
			Check server socket for data received from connected clients
				size_t recv( int sockfd, void *buf, size_t len, int flags );
					sockfd:	The fd referencing the socket from which I'm receiving data
					*buf:	Buffer where data will be saved into
					len:	The length of the data sent
					flags:

			On success returns the length of the message, if a message is too long to fit in the buffer, excess
			bytes may be discarded depending on the type of socket the message is received from.
		*/
		char	buffer[BUFFER_SIZE];
		for	( size_t i = 1; i < MAX_CLIENTS; i++ ) {
			if ( pollFds[i].fd == -1 )
				continue;
			if ( pollFds[i].revents & POLLIN ) {	// Found event
				int	bytesRec = recv( pollFds[i].fd, buffer, sizeof(buffer) -1, 0 );
				if ( bytesRec < 0 )
					std::cerr << "Receive error from client " << pollFds[i].fd << std::endl;
				else if ( bytesRec == 0 ) {
					std::cout << "Client disconnected: " << pollFds[i].fd << std::endl;
					close( pollFds[i].fd );
					pollFds[i].fd = -1;
					if ( i == clients )
						clients--;
				} else {
					buffer[bytesRec] = '\0';
					std::cout << "Received from client " << pollFds[i].fd << ": " << buffer << std::endl;
					// Send data back to the client
					sendData( pollFds, buffer );
				}
			}
		}
	}

	for ( size_t i = 1; i < MAX_CLIENTS; i++ ) {
		if ( pollFds[i].fd == -1 )
			continue;
		else {
			close(pollFds[i].fd);
			pollFds[i].fd = -1;
		}
	}
	close( serverFd );
	return 0;
}

void	sendData( struct pollfd	*pollFds, char *buffer ) {
	for ( size_t i = 1; i < MAX_CLIENTS; i++ ) {
		if ( pollFds[i].fd == -1 )
			continue;
		send( pollFds[i].fd, buffer, strlen(buffer), 0);
	}
}
