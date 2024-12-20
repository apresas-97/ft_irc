#include "Channel.hpp"

Channel::Channel(void) {}

Channel::Channel(const std::string& name) : _name(name), _user_limit(0) 
{
    _modes.insert('i');
    _modes.insert('t');
    _modes.insert('k');
    _modes.insert('o');
    _modes.insert('l');
}

Channel::~Channel(void) {}

// User management
void Channel::addUser(Client& user, bool is_operator) 
{
    if (this->_user_limit != 0 && this->_users.size() >= this->_user_limit)
    {
        throw std::runtime_error("Channel is full");
    }
    this->_users[user.getNickname()] = &user;
    if (is_operator)
    {
        _operators.insert(user.getNickname());
    }
}

void Channel::kickUser(const std::string& userName) 
{
    if (_users.erase(userName) == 0)
    {
        throw std::runtime_error("User not found in channel");
    }
    _operators.erase(userName);
}

Client* Channel::seekUser(const std::string& userName) 
{
    if (_users.count(userName))
    {
        return _users[userName];
    }
    return NULL;
}

bool Channel::isUserInChannel(const std::string& userName) 
{
    return _users.count(userName) > 0;
}

std::set<std::string> Channel::getUsers() const 
{
    std::set<std::string> userList;
    for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it) 
	{
        userList.insert(it->first);
    }
    return userList;
}

std::set<std::string> Channel::getOperators() const 
{
    std::set<std::string> operatorList;
    for (std::set<std::string>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) 
	{
        operatorList.insert(*it);
    }
    return operatorList;
}

void Channel::clearUsers()
{
    _users.clear();
    _operators.clear();
}

// Role management
void Channel::setOperatorStatus(const std::string& userName, bool is_operator) 
{
    if (is_operator) 
        _operators.insert(userName);
    else 
        _operators.erase(userName);
}

// Channel modes
void Channel::setMode(char mode, bool state) 
{
    if (state) 
        _modes.insert(mode);
	else
        _modes.erase(mode);
}

bool Channel::getMode(char mode) const 
{
    return _modes.find(mode) != _modes.end();
}

// Topic and password
std::string Channel::getTopic() const 
{
    return _topic;
}

void Channel::setTopic(const std::string& topic) 
{
    this->_topic = topic;
}

void Channel::setPassword(const std::string& password) 
{
    this->_password = password;
}

bool Channel::validatePassword(const std::string& password) const 
{
    return this->_password == password;
}

// Invitations
void Channel::sendInvite(const std::string& userName) 
{
    _invited_users.insert(userName);
}

bool Channel::isUserInvited(const std::string& userName) 
{
    return _invited_users.find(userName) != _invited_users.end();
}

bool Channel::isUserOperator(const std::string& userName) 
{
    return _operators.find(userName) != _operators.end();
}

// User limit
void Channel::setUserLimit(size_t limit) 
{
    this->_user_limit = limit;
}

size_t	Channel::getUserLimit( void ) {
    return this->_user_limit;
}

size_t	Channel::getUserCount( void ) {
    return this->_user_count;
}
