#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <string>
#include <list>
#include <set>
#include <map>

#include "Client.hpp"

class Client;

typedef struct s_channel_user
{
	Client &	user;
	bool		is_operator;
}				t_channel_user;	

class Channel
{
	private:
		Channel( void );
		Channel( const Channel & src );
		Channel & operator=( const Channel & src );

	public:
		Channel( const std::string & name );
		~Channel( void );

		void	addUser( Client & user, bool is_operator );
		void	kickUser( const std::string & );

		void	toggleInviteOnly( void );

		void	toggleUserLimit( void );
		void	setUserLimit( size_t limit );

		std::string	getTopic( void ) const;
		void		setTopic( const std::string & topic );

		void	setPassword( const std::string & password );
		bool	validatePassword( const std::string & password ) const;
		
		void	promoteUser( const std::string & user_name ); // Turn user to operator
		void	demoteUser( const std::string & user_name ); // Turn operator to user
		void	toggleUserOperator( const std::string & user_name ); // Toggle operator status

		void	sendInvite( const std::string & user_name );
		bool	isUserInvited( const std::string & user_name );

		Client* seekUser( const std::string & );
		// t_channel_user	seekUser( const std::string & user_name ); // Alternative

		bool isUserInChannel( const std::string & userName );
		std::set<std::string> getUsers() const;
		std::set<std::string> getOperators() const;
		void clearUsers();

		void setOperatorStatus(const std::string & userName, bool is_operator);

		bool getMode( char mode ) const;
		void setMode( char mode, bool state );

	private:
		std::string						_name;
		std::string						_topic;
		std::string						_password;
		std::set<char>					_modes;	// Modes (i, t, k, o, l)

		std::map<std::string, Client*>	_users;
		std::set<std::string>			_invited_users;
		std::set<std::string>			_operators;
		size_t							_user_limit;
		size_t							_user_count;

		// bool							_invite_only;
		bool							_has_user_limit;

};
#endif

