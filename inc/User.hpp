#ifndef USER_HPP
#define USER_HPP

#include <iostream>
#include <string>

#define USER_NAME_MAX_LENGTH 9

class User
{
	private:
		bool		validPass;
		std::string	_name;
		std::string	message;

		// bool		_is_operator; // False by default
		/*
			Los users pueden ser operators o regular users, pero eso en realidad
			depende del channel, no del user.
		*/
	public:
};

#endif // USER_HPP