#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <list>

#define SERVER_NAME_MAX_LENGTH 63

class Server {
private:
    std::string         _port;
    std::string         _password;
    int                 _serverFd;
    unsigned int        _clients;
    struct sockaddr_in  _serverAddress;
	struct pollfd		_pollFds[MAX_CLIENTS + 1];

	static Server		*instance;

    void parseInput();
    void initServer();

    void createSocket();
    void bindSocket();
    void configureListening();
    void runServerLoop();
    void handleNewConnections(void);
    void handleClientData(void);
    void sendData(const char *message);
	void setNonBlock(int & socketFd);
	void cleanClose();

	static void signalHandler(int signal);

public:
    Server(const std::string &port, const std::string &password);
    ~Server();
};

#endif // SERVER_HPP
