#include "Channel.hpp"

Channel::Channel(void) : _user_limit(0), _has_user_limit(false)
{}

Channel::Channel(const std::string& name) : _name(name), _user_limit(0), _has_user_limit(false)
{}

Channel::~Channel(void)
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
void Channel::setName(std::string name)
{
    this->_name = name;
}

void Channel::setTopic(const std::string& topic)
{
    this->_topic = topic;
}

void Channel::setKey(std::string key)
{
    this->_key = key;
}

void Channel::setMode(char mode, bool state)
{
    std::vector<char>::iterator it = std::find(_modes.begin(), _modes.end(), mode);
    if (state)
    {
        // Si `mode` no está ya en `_modes`, lo agregamos
        if (it == _modes.end())
            _modes.push_back(mode);
    }
    else
    {
        // Si `mode` está en `_modes`, lo eliminamos
        if (it != _modes.end())
            _modes.erase(it);
    }
}
void Channel::setUserLimit(size_t limit)
{
    this->_user_limit = limit;
    this->_has_user_limit = true;
}

// Getters
std::string Channel::getName(void) const
{
    return _name;
}

std::string Channel::getTopic(void) const
{
    return _topic;
}

std::string Channel::getKey(void) const
{
    return _key;
}

bool Channel::getMode(char mode) const
{
    return std::find(_modes.begin(), _modes.end(), mode) != _modes.end();
}

std::vector<char>	Channel::getModes( void ) const
{
	return this->_modes;
}

std::string Channel::getModesString( void ) const // Tests pending
{
    std::string modes;
    for (size_t i = 0; i < _modes.size(); i++)
    {
        modes += _modes[i];
    }
    return modes;
}

std::string Channel::getModesParameters( void ) const // Tests pending
{
    // Alphabetic order of parametered modes, for reference: k l
    std::string key = _key;
    std::string limit = uLongToString(_user_limit);

    bool k_set = this->getMode('k');
    bool l_set = this->getMode('l');
    size_t k_index = 0;
    size_t l_index = 0;
    for (size_t i = 0; i < _modes.size(); i++)
    {
        if (_modes[i] == 'k')
            k_index = i;
        else if (_modes[i] == 'l')
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

size_t Channel::getUserLimit() const
{
    return this->_user_limit;
}

size_t Channel::getUserCount() const
{
    return this->_users.size();
}

std::vector<std::string> Channel::getUsers() const
{
    std::vector<std::string> users;
    for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it)
    {
        users.push_back(it->first);
    }
    return users;
}

std::map<std::string, Client *>	Channel::getTrueUsers() const
{
	return this->_users;
}

std::vector<std::string> Channel::getOperators() const
{
    std::vector<std::string> operators;
    for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        operators.push_back(it->first);
    }
    return operators;
}

std::map<std::string, Client *>	Channel::getTrueOperators() const
{
	return this->_operators;
}

std::vector<std::string> Channel::getInvitedUsers() const
{
    std::vector<std::string> invitedUsers;
    for (std::map<std::string, Client*>::const_iterator it = _invited_users.begin(); it != _invited_users.end(); ++it)
    {
        invitedUsers.push_back(it->first);
    }
    return invitedUsers;
}

std::map<std::string, Client *>	Channel::getTrueInvitedUsers() const
{
	return this->_invited_users;
}

std::vector<int> Channel::getFds(std::string key) const 
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
void Channel::addUser(Client& user, bool is_operator)
{
    if (_has_user_limit && _users.size() >= this->_user_limit)
    {
        throw std::runtime_error("Channel is full");
    }

    this->_users[user.getNickname()] = &user;

    if (is_operator)
    {
        if (_operators.find(user.getNickname()) == _operators.end())
        {
            _operators[user.getNickname()] = &user;
        }
    }
}

void Channel::kickUser(const std::string& userName)
{
    if (_users.erase(userName) == 0)
    {
        throw std::runtime_error("User not found in channel");
    }

    if (_operators.erase(userName) == 0) {}
}

void Channel::promoteUser(const std::string& userName)
{
    if (_users.find(userName) != _users.end())
    {
        if (_operators.find(userName) == _operators.end())
        {
            _operators[userName] = _users[userName];
        }
    } 
    else
    {
        throw std::runtime_error("User not found in channel");
    }
}

void Channel::demoteUser(const std::string& userName)
{
    if (_operators.erase(userName) == 0)
    {
        throw std::runtime_error("User is not an operator");
    }
}

void Channel::inviteUser(const std::string& userName)
{
    if (_invited_users.find(userName) == _invited_users.end())
    {
        _invited_users[userName] = _users[userName];
    }
    else
    {
        throw std::runtime_error("User is already invited");
    }
}

void Channel::uninviteUser(const std::string& userName)
{
    if (_invited_users.erase(userName) == 0)
    {
        throw std::runtime_error("User is not invited");
    }
}

// User Localizers
bool Channel::isUserInChannel(const std::string& userName)
{
    return _users.find(userName) != _users.end();
}

bool Channel::isUserOperator(const std::string& userName)
{
    return _operators.find(userName) != _operators.end();
}

bool Channel::isUserInvited(const std::string& userName)
{
    return _invited_users.find(userName) != _invited_users.end();
}

bool Channel::isEmpty(void) const
{
    return _users.empty() && _operators.empty();
}
