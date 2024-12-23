#include "Channel.hpp"

Channel::Channel(void) : _user_limit(0), _has_user_limit(false)
{}

Channel::Channel(const std::string& name) : _name(name), _user_limit(0), _has_user_limit(false)
{}

Channel::~Channel(void)
{}

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

std::vector<std::string> Channel::getOperators() const
{
    std::vector<std::string> operators;
    for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it)
    {
        operators.push_back(it->first);
    }
    return operators;
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

std::vector<int> Channel::getFds(std::string key) const {
    std::vector<int> fds;

    if (key == "users") {
        for (std::map<std::string, Client*>::const_iterator it = _users.begin(); it != _users.end(); ++it) {
            int fd = it->second->getSocket();
            if (fd != -1) {
                fds.push_back(fd);
            }
        }
    }
    else if (key == "operators") {
        for (std::map<std::string, Client*>::const_iterator it = _operators.begin(); it != _operators.end(); ++it) {
            int fd = it->second->getSocket();
            if (fd != -1) {
                fds.push_back(fd);
            }
        }
    }
    else if (key == "invited") {
        for (std::map<std::string, Client*>::const_iterator it = _invited_users.begin(); it != _invited_users.end(); ++it) {
            int fd = it->second->getSocket();
            if (fd != -1) {
                fds.push_back(fd);
            }
        }
    }

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
