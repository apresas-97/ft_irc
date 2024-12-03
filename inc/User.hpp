#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <string>

#define USER_NAME_MAX_LENGTH 9

class User
{
	public:

	private:
		std::string	_name;
		// bool		_is_operator; // False by default
		/*
			Los users pueden ser operators o regular users, pero eso en realidad
			depende del channel, no del user.
		*/
};

#endif // USER_HPP