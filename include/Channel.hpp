#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <list>

#include "Client.hpp"

/*
	Channels are created implicitly when a user joins it,
	they are destroyed when the last user leaves it.
*/

#define CHANNEL_NAME_MAX_LENGTH 200
// Channel names can be up to 200 characters long
// They must start with a '#' or '&' character
// They can't contain spaces or commas or "control G" characters (idk what that last one is yet)

/*
Whoever creates the channel is automatically an operator
*/

/*
Channel operators are distinguished by the '@' symbol before their nickname
*/

class User;

typedef struct s_channel_user
{
	User &	user;
	bool	is_operator;
}				t_channel_user;	

class Channel
{
	private:
		Channel( void );
		Channel( const Channel & src );
		Channel & operator=( const Channel & src );

	public:
		Channel( const std::string & name );
		~Channel();

		void	addUser( User & user );
		void	addUser( User & user, bool is_operator );
		void	kickUser( const std::string & user_name );

		void	toggleInviteOnly( void );

		void	toggleUserLimit( void );
		void	setUserLimit( size_t limit );

		std::string	getTopic( void ) const;
		void		setTopic( const std::string & topic );

		void	setPassword( const std::string & password );

		void	promoteUser( const std::string & user_name ); // Turn user to operator
		void	demoteUser( const std::string & user_name ); // Turn operator to user
		void	toggleUserOperator( const std::string & user_name ); // Toggle operator status

		void	sendInvite( const std::string & user_name );

		User	seekUser( const std::string & user_name );
		// t_channel_user	seekUser( const std::string & user_name ); // Alternative

	private:
		std::list<User>	_users;
		// std::list<t_channel_user>	_users; // Alternative

		bool	_invite_only;
		bool	_has_user_limit;

		size_t	_user_limit;
		size_t	_user_count;

		std::string	_password;
		std::string	_topic;

};

#endif // CHANNEL_HPP