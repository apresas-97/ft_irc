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
				pollfd:		
				nfds_t:		
				timeout:	Specifies the number of milliseconds that poll should block waiting for a fd to becomre ready

			Returns the number of elements in pollFds whose revents fields have been set to a non-zero value aka even or error
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
		} else

		/*
			Check server socket for new client connections
				accept( )...
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
			for ( size_t i = 1; i < MAX_CLIENTS; i++ ) {
				if ( pollFds[i].fd == -1 ) {
					pollFds[i].fd = clientFd;
					std::cout << "New client connection: " << clientFd << std::endl;
					clientAdded = true;
					clients++;
					break ;
				}
			}
			if ( !clientAdded ) {	// Unable to add new client, max connections reached
				std::cerr << "Unable to add new client, max number of connections reached" << std::endl;
				close( clientFd );
			}
		}

		/*
			Check server socket for data from connected clients
				recv( )...
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
