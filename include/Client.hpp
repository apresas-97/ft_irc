#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <iostream>
#include <string>

#define USER_NAME_MAX_LENGTH 9

class Client
{
	public:

	private:
		std::string	_nickname;
		std::string	_real_name;
		/*
			The IRC protocol says we need:
			- Nickname (Unique, max 9 characters)
			- Real name of the host (Unsure of what this means)
			- Username of client in that hose
			- Server to which the client is connected 
		*/
};

#endif // CLIENT_HPP