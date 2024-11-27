#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>

class Server
{
private:
	Server( void ); // No queremos un Server default constructed
	Server( const Server & src ); // Tampoco nos interesa copiar un Server

public:
	~Server();

	Server & operator=( const Server & src );

private:
	std::string &	_password;

};

#endif // SERVER_HPP