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
		Channel(const std::string& name);
		~Channel( void );

		// User management
		void addUser(Client& user, bool is_operator);
		void kickUser(const std::string& userName);
		Client* seekUser(const std::string& userName);
		bool isUserInChannel(const std::string& userName);
		std::set<std::string> getUsers() const;
		std::set<std::string> getOperators() const;
		void clearUsers();

		// Role management
		void setOperatorStatus(const std::string& userName, bool is_operator);

		// Channel modes
		void setMode(char mode, bool state);
		bool getMode(char mode) const;

		// Topic and password
		std::string getTopic() const;
		void setTopic(const std::string& topic);
		void setPassword(const std::string& password);
		bool validatePassword(const std::string& password) const;

		// Invitations
		void sendInvite(const std::string& userName);
		bool isUserInvited(const std::string& userName);

		// User limit
		void setUserLimit(size_t limit);

	private:

		std::string						_name;
		std::string						_topic;
		std::string						_password;
		std::set<char>					_modes;	// Modes (i, t, k, o, l)

		std::map<std::string, Client*>	_users;
		std::set<std::string>			_invitedUsers;
		std::set<std::string>			_operators;
		size_t							_userLimit;
		size_t							_userCount;

		// bool							_inviteOnly;
		bool							_hasUserLimit;
};

#endif
