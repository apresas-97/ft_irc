
#include "Channel.hpp"

Channel::Channel(const std::string& name) : _name(name), _userLimit(-1) {
    _modes = {'i', 't', 'k', 'o', 'l'};  // Modes are initialized as active
}

Channel::~Channel(void) {}

// User management
void Channel::addUser(Client& user, bool is_operator) {
    if (this->_userLimit != -1 && this->_users.size() >= this->_userLimit) {
        throw std::runtime_error("Channel is full");
    }
    this->_users[user.getNickname()] = &user;
    if (is_operator) {
        _operators.insert(user.getNickname());
    }
} 

void Channel::kickUser(const std::string& user_name) {
    if (_users.erase(user_name) == 0) {
        throw std::runtime_error("User not found in channel");
    }
    _operators.erase(user_name);
}

Client* Channel::seekUser(const std::string& user_name) {
    std::map<std::string, Client*>::iterator it = _users.find(user_name);
    return (it != _users.end()) ? it->second : nullptr;
}

bool Channel::isUserInChannel(const std::string& user_name) {
    return _users.find(user_name) != _users.end();
}

std::set<std::string> Channel::getUsers() const {
    std::set<std::string> userList;
    for (const auto& pair : _users) {
        userList.insert(pair.first);
    }
    return userList;
}

std::set<std::string> Channel::getOperators() const {
    return std::set<std::string>(_operators.begin(), _operators.end());
}

void Channel::clearUsers() {
    _users.clear();
    _operators.clear();
}

// Role management
void Channel::setOperatorStatus(const std::string& user_name, bool is_operator) {
    if (is_operator) {
        _operators.insert(user_name);
    } else {
        _operators.erase(user_name);
    }
}

// Channel modes
void Channel::setMode(char mode, bool state) {
    if (state) {
        _modes.insert(mode);
    } else {
        _modes.erase(mode);
    }
}

bool Channel::getMode(char mode) const {
    return _modes.find(mode) != _modes.end();
}

// Topic and password
std::string Channel::getTopic() const {
    return _topic;
}

void Channel::setTopic(const std::string& topic) {
    this->_topic = topic;
}

void Channel::setPassword(const std::string& password) {
    this->_password = password;
}

bool Channel::validatePassword(const std::string& password) const {
    return this->_password == password;
}

// Invitations
void Channel::sendInvite(const std::string& user_name) {
    _invitedUsers.insert(user_name);
}

bool Channel::isUserInvited(const std::string& user_name) {
    return _invitedUsers.find(user_name) != _invitedUsers.end();
}

// User limit
void Channel::setUserLimit(size_t limit) {
    this->_userLimit = limit;
}