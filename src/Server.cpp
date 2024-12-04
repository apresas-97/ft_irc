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

#include "ft_irc.hpp"
#include "Server.hpp"

#define PORT 8080 // Port number to bind

Server*	Server::instance = NULL;

void Server::signalHandler(int signal) {
	if (signal == SIGINT) {
		if (instance)
			instance->cleanClose();
		exit(0);
	}
}

void Server::cleanClose() {
	std::cout << "call clean close" << std::endl;
	close(_serverFd);
	for (size_t i = 1; i < MAX_CLIENTS; i++) {
		if (_pollFds[i].fd == -1)
			continue;
		close(_pollFds[i].fd);
	}
}

Server::Server(const std::string &port, const std::string &password) : _port(port), _password(password){
	instance = this;
	parseInput();
	initServer();
}

Server::~Server() {
	if (_serverFd != -1) {
		std::cout << "Server destructor called" << std::endl;
		close(_serverFd);
	}
}

void Server::parseInput() {

	unsigned int		port;
	std::string			port_str(this->_port);
	std::istringstream	iss(this->_port);

	iss >> port;
	if (this->_port.empty() || iss.fail() || !iss.eof())
		throw std::invalid_argument("Invalid port input received");
	if (port < 1024 || port > 65535)
		throw std::invalid_argument("Port must be between 1024 and 49151");
	std::string password(this->_password);
}

void Server::initServer() {
	if (signal(SIGINT, signalHandler) == SIG_ERR)
		throw std::runtime_error("Error settig up signal handler");
	createSocket();
	bindSocket();
	configureListening();
	runServerLoop();
}

void Server::createSocket() {
	_serverFd = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverFd < 0) {
		throw std::runtime_error("Server socket creation failed");
	}
	setNonBlock(_serverFd);
}

void Server::setNonBlock(int & socketFd) {
	int	flags = fcntl( socketFd, F_GETFL, 0 );
	if (flags < 0) {
		cleanClose();
		throw std::runtime_error("Error setting socket to non-blocking");
	}
	flags |= O_NONBLOCK;
	if (fcntl(socketFd, F_SETFL, flags) == -1 ) {
		cleanClose();
		throw std::runtime_error("Error setting socket to non-blocking");
	}
}

void Server::bindSocket() {
	int					port;
	std::istringstream	iss(this->_port);

	iss >> port;
    memset(&this->_serverAddress, 0, sizeof(this->_serverAddress));
    this->_serverAddress.sin_family = AF_INET;
    this->_serverAddress.sin_addr.s_addr = INADDR_ANY;
    this->_serverAddress.sin_port = htons(port);

	if (bind(_serverFd, (struct sockaddr *)&this->_serverAddress, sizeof(this->_serverAddress)) < 0) {
		close(_serverFd);
		throw std::runtime_error("Server socket binding failed");
	}
}

void Server::configureListening() {
    if (listen(_serverFd, MAX_CLIENTS) < 0) {
        close(_serverFd);
        throw std::runtime_error("Listen failed");
    }
    std::cout << "Server is listening on port " << _port << std::endl;
}

void Server::runServerLoop() {
    _pollFds[0].fd = _serverFd;
    _pollFds[0].events = POLLIN;

    for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
        _pollFds[i].fd = -1;
        _pollFds[i].events = POLLIN;
    }
	_clients = 0;
    std::cout << "Server started, waiting for clients..." << std::endl;

    while (true) {
        int pollCount = poll(_pollFds, _clients + 1, TIMEOUT);
        if (pollCount < 0) {
            std::cerr << "Poll error: " << strerror(errno) << std::endl;
            break;
        } else if (pollCount == 0) {
            std::cout << "Poll timed out, no activity" << std::endl;
            continue;
        }

        handleNewConnections();
        handleClientData();
    }
}

void Server::handleNewConnections() {
	if (_pollFds[0].revents & POLLIN) {
		struct sockaddr_in clientAddress;
		socklen_t addressLen = sizeof(clientAddress);
		int clientFd = accept(_serverFd, (struct sockaddr *)&clientAddress, &addressLen);

		if (clientFd < 0) {
			std::cerr << "Failed to accept new client" << std::endl;
			return;
		}

		bool clientAdded = false;
		for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
			if (_pollFds[i].fd == -1) {
				_pollFds[i].fd = clientFd;
				std::cout << "New client connected: " << clientFd << std::endl;
				clientAdded = true;
				if (i > _clients)
					_clients++;
				break;
			}
		}

		if (!clientAdded) {
			std::cerr << "Max clients reached, closing connection" << std::endl;
			close(clientFd);
		}
	}
}

void Server::handleClientData() {
	char buffer[BUFFER_SIZE];
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
		if (_pollFds[i].fd == -1)
			continue;

		if (_pollFds[i].revents & POLLIN) {
			int bytesReceived = recv(_pollFds[i].fd, buffer, sizeof(buffer) - 1, 0);
			if (bytesReceived < 0) {
				std::cerr << "Error receiving data from client " << _pollFds[i].fd << std::endl;
			} else if (bytesReceived == 0) {
				std::cout << "Client disconnected: " << _pollFds[i].fd << std::endl;
				close(_pollFds[i].fd);
				_pollFds[i].fd = -1;
				if (i == _clients)
					_clients--;
			} else {
				buffer[bytesReceived] = '\0';
				std::cout << "Received from client " << _pollFds[i].fd << ": " << buffer;
				sendData(buffer);
			}
		}
	}
}

void Server::sendData(const char *message) {
	for (size_t i = 1; i < MAX_CLIENTS + 1; i++) {
		if (_pollFds[i].fd != -1) {
			send(_pollFds[i].fd, message, strlen(message), 0);
		}
	}
}
