#include "Client.hpp"

Client::Client( int fd, struct sockaddr_storage address ) : _registered(false), _authorised(false) {
	this->_address = address;
}

Client::~Client( void ) {
	// ?
}

const std::string & Client::getNickname( void ) const
{
	return this->_nickname;
}

const std::string & Client::getUsername( void ) const
{
	return this->_username;
}

const std::string & Client::getHostname( void ) const
{
	return this->_hostname;
}

const std::string & Client::getRealname( void ) const
{
	return this->_realname;
}
