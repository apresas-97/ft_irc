#include "Client.hpp"

Client::Client( void ) {}

// Client::Client( int fd, struct sockaddr_storage address ) : _registered(false), _authorised(false) {
// 	this->_address = address;
// }

Client::Client( int socket ) : _registered(false), _authorised(false) {
	this->_socket = socket;
}

Client::~Client( void ) {}


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

const int & Client::getSocket( void ) const {
	return this->_socket;
}


void Client::setNickname( const std::string & nickname ) 
{
	this->_nickname = nickname;
}

void Client::setUsername( const std::string & username ) 
{
	this->_username = username;
}

void Client::setHostname( const std::string & hostname ) 
{
	this->_hostname = hostname;
}

void Client::setRealname( const std::string & realname ) 
{
	this->_realname = realname;
}

void Client::setSocket( const int & socket ) {
	this->_socket = socket;
}

std::string Client::getPrefix( void ) const
{
	std::string	prefix;

	prefix = ":" + this->getUserIdentifier();
	return prefix;
}

std::string Client::getUserIdentifier( void ) const
{
	std::string	identifier;

	identifier += this->getNickname();
	identifier += "!";
	identifier += this->getUsername();
	identifier += "@";
	identifier += this->getHostname();

	return identifier;
}

bool Client::isAuthorised( void ) const
{
	return this->_authorised;
}

void Client::setAuthorised( bool value )
{
	this->_authorised = value;
}

bool Client::isRegistered( void ) const
{
	return this->_registered;
}

void Client::setRegistered( bool value )
{
	this->_registered = value;
}

bool Client::getMode( char mode ) const 
{
	switch (mode)
	{
		case 'a':
			return this->_mode.a;
		case 'i':
			return this->_mode.i;
		case 'w':
			return this->_mode.w;
		case 'r':
			return this->_mode.r;
		case 'o':
			return this->_mode.o;
		case 'O':
			return this->_mode.O;
		case 's':
			return this->_mode.s;
		default:
			return false;
	}
}

void Client::setMode( char mode, bool value )
{
	switch (mode)
	{
		case 'a':
			this->_mode.a = value;
			break;
		case 'i':
			this->_mode.i = value;
			break;
		case 'w':
			this->_mode.w = value;
			break;
		case 'r':
			this->_mode.r = value;
			break;
		case 'o':
			this->_mode.o = value;
			break;
		case 'O':
			this->_mode.O = value;
			break;
		case 's':
			this->_mode.s = value;
			break;
		default:
			break;
	}
}

