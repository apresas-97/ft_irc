#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "Client.hpp"

class Client;

class Channel
{
	private:

	public:
		Channel( void );
		Channel( const std::string & name );
		~Channel( void );
		Channel( const Channel & src );
		Channel & operator=( const Channel & src );

		// Setters
        void	setName(std::string name);
		void	setTopic( const std::string & topic );
        void	setKey(std::string key);
		void	setMode( char mode, bool state );
		void	setUserLimit( size_t limit );

		// Getters
        std::string				getName( void ) const;
		std::string				getTopic( void ) const;
        std::string				getKey( void ) const;
		bool					getMode( char mode ) const;
		std::vector<char>		getModes( void ) const;
		std::string				getModesString( void ) const;
		std::string				getModesParameters( void ) const;
		size_t					getUserLimit( void ) const;
		size_t					getUserCount( void ) const;

		std::vector<std::string>	getUsers( void ) const;
		std::map<std::string, Client*>	getTrueUsers( void ) const;
		std::vector<std::string>	getOperators( void ) const;
		std::map<std::string, Client*>	getTrueOperators( void ) const;
		std::vector<std::string>	getInvitedUsers( void ) const;
		std::map<std::string, Client*>	getTrueInvitedUsers( void ) const;

		std::vector<int>			getFds(std::string key) const;

		// User Management
		void	addUser( Client & user, bool is_operator );
		void	kickUser( const std::string & );
		void	promoteUser( const std::string & userName );	// Turn user to operator
		void	demoteUser( const std::string & userName );	// Turn operator to user
		void	inviteUser( const std::string & userName );	// Invite user
		void	uninviteUser( const std::string & userName );	// Uninvite user

		// User Characteristics
		bool	isUserInChannel( const std::string & userName );
		bool	isUserOperator( const std::string & userName );
		bool	isUserInvited( const std::string & userName );

	private:
		std::string						_name;
		std::string						_topic;
		std::string						_key;
		std::vector<char>				_modes;	// Modes (i, t, k, o, l)

		std::map<std::string, Client*>	_users;
		std::map<std::string, Client*>	_invited_users;
		std::map<std::string, Client*>	_operators;
		size_t							_user_limit;
		bool							_has_user_limit;

};

#endif // CHANNEL_HPP
