#include "Channel.hpp"

Channel::Channel( void ) : _user_limit(-1), _has_user_limit(false)
{}

Channel::Channel( const std::string& name ) : _name(name), _user_limit(-1), _has_user_limit(false)
{}

Channel::~Channel( void )
{}

Channel & Channel::operator=( const Channel & src )
{
	if ( this != &src ) 
	{
		this->setName(src.getName());
		this->setTopic(src.getTopic());
		this->setKey(src.getKey());
		this->_modes = src.getModes();
		this->_users = src.getTrueUsers();
		this->_invited_users = src.getTrueInvitedUsers();
		this->_operators = src.getTrueOperators();
		if (src.getUserLimit())
			this->setUserLimit(src.getUserLimit());
		else
		{
			this->_has_user_limit = false;
			this->_user_limit = 0;
		}
	}
	return *this;
}


Channel::Channel( const Channel & src )
{
	*this = src;
}

// Setters
void Channel::setName( std::string name )
{
	this->_name = name;
}

void Channel::setTopic( const std::string & topic )
{
	this->_topic = topic;
}

void Channel::setKey( std::string key )
{
	this->_key = key;
}

void Channel::setMode( char mode, bool state )
{
	std::cout << "setMode called, mode = '" << mode << "', state = " << state << std::endl;
	std::vector<char>::iterator it = std::find(this->_modes.begin(), this->_modes.end(), mode);
	if (state)
	{
		// Si `mode` no está ya en `_modes`, lo agregamos
		if (it == this->_modes.end())
		{
			std::cout << "Adding mode '" << mode << "'" << std::endl;
			this->_modes.push_back(mode);
		}
	}
	else
	{
		// Si `mode` está en `_modes`, lo eliminamos
		if (it != this->_modes.end())
		{
			std::cout << "Removing mode '" << mode << "'" << std::endl;
			this->_modes.erase(it);
		}
	}
}

void Channel::setUserLimit( size_t limit )
{
	this->_user_limit = limit;
	this->_has_user_limit = true;
}

// Getters
std::string Channel::getName( void ) const
{
	return this->_name;
}

std::string Channel::getTopic( void ) const
{
	return this->_topic;
}

std::string Channel::getKey( void ) const
{
	return this->_key;
}

bool Channel::getMode( char mode ) const
{
	return std::find(this->_modes.begin(), this->_modes.end(), mode) != this->_modes.end();
}

std::vector<char>	Channel::getModes( void ) const
{
	return this->_modes;
}

std::string Channel::getModesString( void ) const // Tests pending
{
	std::string modes;
	for (size_t i = 0; i < this->_modes.size(); i++)
	{
		modes += this->_modes[i];
	}
	return modes;
}

std::string Channel::getModesParameters( void ) const // Tests pending
{
	// Alphabetic order of parametered modes, for reference: k l
	std::string key = this->_key;
	std::string limit = uLongToString(_user_limit);

	bool k_set = this->getMode('k');
	bool l_set = this->getMode('l');
	size_t k_index = 0;
	size_t l_index = 0;
	for (size_t i = 0; i < this->_modes.size(); i++)
	{
		if (this->_modes[i] == 'k')
			k_index = i;
		else if (this->_modes[i] == 'l')
			l_index = i;
	}
	if (k_set && l_set)
	{
		if (k_index < l_index)
			return key + " " + limit;
		else
			return limit + " " + key;
	}
	else if (k_set)
		return key;
	else if (l_set)
		return limit;
	return "";
}

size_t Channel::getUserLimit( void ) const
{
	return this->_user_limit;
}

size_t Channel::getUserCount( void ) const
{
	return this->_users.size();
}

std::vector<std::string> Channel::getUsers( void ) const
{
	std::vector<std::string> users;
	for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it)
	{
		users.push_back(it->first);
	}
	return users;
}

std::map<std::string, Client *>	Channel::getTrueUsers( void ) const
{
	return this->_users;
}

std::vector<std::string> Channel::getOperators( void ) const
{
	std::vector<std::string> operators;
	for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it)
	{
		operators.push_back(it->first);
	}
	return operators;
}

std::map<std::string, Client *>	Channel::getTrueOperators( void ) const
{
	return this->_operators;
}

std::vector<std::string> Channel::getInvitedUsers( void ) const
{
	std::vector<std::string> invitedUsers;
	for (std::map<std::string, Client*>::const_iterator it = _invited_users.begin(); it != _invited_users.end(); ++it)
	{
		invitedUsers.push_back(it->first);
	}
	return invitedUsers;
}

std::map<std::string, Client *>	Channel::getTrueInvitedUsers( void ) const
{
	return this->_invited_users;
}

std::vector<int> Channel::getFds( std::string key ) const 
{
	std::vector<int> fds;

	if (key == "users") 
	{
		for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it) 
		{
			int fd = it->second->getSocket();
			if (fd != -1) {
				fds.push_back(fd);
			}
		}
	}
	else if (key == "operators") 
	{
		for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) 
		{
			int fd = it->second->getSocket();
			if (fd != -1) {
				fds.push_back(fd);
			}
		}
	}
	else if (key == "invited") 
	{
		for (std::map<std::string, Client*>::const_iterator it = _invited_users.begin(); it != _invited_users.end(); ++it) 
		{
			int fd = it->second->getSocket();
			if (fd != -1) {
				fds.push_back(fd);
			}
		}
	}

	return fds;
}

std::set<int>   Channel::getFdsSet( std::string key ) const
{
	std::vector<int> fdsVector = this->getFds(key);
	std::set<int> fds;

	for (std::vector<int>::const_iterator it = fdsVector.begin(); it != fdsVector.end(); ++it)
		fds.insert(*it);
	return fds;
}

// User Management
void Channel::addUser( Client * user, bool is_operator )
{
	std::cout << "Adding user \"" << user->getNickname() << "\" to channel \"" << this->_name << "\"" << std::endl;
	std::cout << "User count before: " << _users.size() << std::endl;
	if (_has_user_limit && _users.size() >= this->_user_limit)
	{
		// TODO PROBABLY DONT THROW AN EXCEPTION HERE
		throw std::runtime_error("Channel is full");
	}

	std::cout << getName() << std::endl;
	this->_users.insert(std::pair<std::string, Client*>(user->getNickname(), user));
	std::cout << "User count after: " << _users.size() << std::endl;

	std::cout << "Users in channel:" << std::endl;
	for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it)
	{
		std::cout << it->first << std::endl;
	}
	std::cout << "User list end" << std::endl;

	if (is_operator)
	{
		if (_operators.find(user->getNickname()) == _operators.end())
		{
			this->_operators.insert(std::pair<std::string, Client *>(user->getNickname(), user));
		}
	}
}

void Channel::kickUser( const std::string & nickname )
{
	if (this->_users.erase(nickname) == 0)
		throw std::runtime_error("User not found in channel");
	if (this->_operators.erase(nickname) == 0) {}
}

void Channel::promoteUser( const std::string & nickname )
{
	if (_users.find(nickname) != _users.end())
	{
		if (_operators.find(nickname) == _operators.end())
		{
			_operators[nickname] = _users[nickname];
		}
	} 
	else
	{
		throw std::runtime_error("User not found in channel");
	}
}

void Channel::demoteUser( const std::string & nickname )
{
	if (_operators.erase(nickname) == 0)
	{
		throw std::runtime_error("User is not an operator");
	}
}

void Channel::inviteUser( const std::string & nickname )
{
	if (_invited_users.find(nickname) == _invited_users.end())
	{
		_invited_users[nickname] = _users[nickname];
	}
	else
	{
		throw std::runtime_error("User is already invited");
	}
}

void Channel::uninviteUser( const std::string & nickname )
{
	if (_invited_users.erase(nickname) == 0)
	{
		throw std::runtime_error("User is not invited");
	}
}

#include <iostream>
#include <map>
#include <string>
// User Localizers
bool Channel::isUserInChannel( const std::string & nickname )
{
	for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it)
	{
/*
		if (it != _users.end())  // Verificación de rango
		{
			if (it->first.empty() || it->second == NULL) continue;  // Verificación adicional para evitar datos nulos o vacíos
			RED_TEXT(it->first);      // Nombre del usuario
			RED_TEXT(it->second);     // Cliente asociado
			RED_TEXT(nickname);       // Nickname del usuario buscado
		}
		else 
		{
			std::cout << "Iterador fuera de rango" << std::endl;
		}
*/
		if (it->first == nickname)
		{
			return true;
		}
	}
	return false;
}

bool Channel::isUserOperator( const std::string & nickname )
{
	for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (it->first == nickname)
			return true;
	}
	return false;
}

bool Channel::isUserInvited( const std::string & nickname )
{
	for (std::map<std::string, Client*>::const_iterator it = _invited_users.begin(); it != _invited_users.end(); ++it)
	{
		if (it->first == nickname)
			return true;
	}
	return false;
}

bool Channel::isEmpty( void ) const
{
	return _users.empty();
}
