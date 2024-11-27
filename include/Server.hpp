#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <list>

#define SERVER_NAME_MAX_LENGTH 63

class Server
{
private:
	Server( void ); // No queremos un Server default constructed
	Server( const Server & src ); // Tampoco nos interesa copiar un Server

public:
	~Server();

	Server & operator=( const Server & src );

private:
	std::string	_name; // server name, max length of 63 characters
	std::string &	_password;

	// Channels ?
	// std::list<Channel>	_channels;

	// Users ?
	// std::list<User>	_users;

};

#endif // SERVER_HPP