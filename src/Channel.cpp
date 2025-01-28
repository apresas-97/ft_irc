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
	std::vector<char>::iterator it = std::find(this->_modes.begin(), this->_modes.end(), mode);
	if (state)
	{
		// Si `mode` no está ya en `_modes`, lo agregamos
		if (it == this->_modes.end())
		{
			this->_modes.push_back(mode);
		}
	}
	else
	{
		// Si `mode` está en `_modes`, lo eliminamos
		if (it != this->_modes.end())
		{
			this->_modes.erase(it);
		}
	}
}

void Channel::setUserLimit( long limit )
{
	this->_user_limit = limit;
	if (limit > 0)
		this->_has_user_limit = true;
	else
		this->_has_user_limit = false;
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

std::vector<std::string> Channel::getUsersOpClean( void ) const
{
	std::vector<std::string> users;
	for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (isUserOperator(it->first))
		{
			users.push_back("@" + it->first);
		}
		else
			users.push_back(it->first);
	}
	return users;
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
	if (_has_user_limit && static_cast<long>(_users.size()) >= this->_user_limit)
		RED_TEXT("AddUser failed: attempted to add a user past the limit.");

	this->_users.insert(std::pair<std::string, Client*>(user->getNickname(), user));

	if (is_operator)
		if (_operators.find(user->getNickname()) == _operators.end())
			this->_operators.insert(std::pair<std::string, Client *>(user->getNickname(), user));
}

void Channel::kickUser( const std::string & nickname )
{
	if (this->_users.erase(nickname) == 0)
		RED_TEXT("Kick failed: user was not found.");
	if (this->_operators.erase(nickname) == 0) {}
}

void Channel::promoteUser( const std::string & nickname )
{
	if (_users.find(nickname) != _users.end())
	{
		if (_operators.find(nickname) == _operators.end())
			_operators[nickname] = _users[nickname];
	} 
	else
	{
		RED_TEXT("PromoteUser failed: User was not found.");
	}
}

void Channel::demoteUser( const std::string & nickname )
{
	if (_operators.erase(nickname) == 0)
	{
		RED_TEXT("DemoteUser failed: user is not an operator.");
	}
}

void Channel::inviteUser( const std::string & nickname, Client * client )
{
	if (_invited_users.find(nickname) == _invited_users.end())
	{
		_invited_users[nickname] = client;
	}
	else
	{
		RED_TEXT("InviteUser failed: User is already invited.");
	}
}

void Channel::uninviteUser( const std::string & nickname )
{
	_invited_users.erase(nickname);
}

void Channel::updateNickname( const std::string oldname, const std::string newname )
{
	std::map<std::string, Client*>::iterator	it;
	Client	*	client;

	it = _users.find(oldname);
	if (it != _users.end())
	{
		client = it->second;
		_users.erase(it);
		_users[newname] = client;
	}
	it = _invited_users.find(oldname);
	if (it != _invited_users.end())
	{
		client = it->second;
		_invited_users.erase(it);
		_invited_users[newname] = client;
	}
	it = _operators.find(oldname);
	if (it != _operators.end())
	{
		client = it->second;
		_operators.erase(it);
		_operators[newname] = client;
	}
}

#include <iostream>
#include <map>
#include <string>
// User Localizers
bool Channel::isUserInChannel( const std::string & nickname ) const
{
	for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->first == nickname)
		{
			return true;
		}
	}
	return false;
}

bool Channel::isUserOperator( const std::string & nickname ) const
{
	for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it)
	{
		if (it->first == nickname)
		{
			return true;
			break ;
		}
	}
	return false;
}

bool Channel::isUserInvited( const std::string & nickname ) const
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
	std::cout << _users.size() << std::endl;
	return _users.empty();
}
