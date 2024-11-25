#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>

class Server
{
private:
	Server( void );
	Server( const Server & src );

public:
	~Server();

	Server & operator=( const Server & src );

private:
	std::string &	_password;

};

#endif // SERVER_HPP