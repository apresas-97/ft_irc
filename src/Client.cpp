#include "Client.hpp"
#include <cstring>
#include <algorithm>

void	Client::initModes( void )
{
	this->setMode('a', false);
	this->setMode('i', false);
	this->setMode('w', false);
	this->setMode('r', false);
	this->setMode('o', false);
	this->setMode('O', false);
	this->setMode('s', false);
}

// Constructors and Destructor
Client::Client(void)
{
	memset(_buffer, 0, BUFFER_SIZE);
	_authorised = false;
	_registered = false;
	_terminate = false;
	_hostname_looked_up = false;
	_chan_limit = MAX_CLIENT_CHANNELS;
	_chan_count = 0;
	_last_activity = std::time(NULL);
	_pong_timer = std::time(NULL);
	_expected_pong = false;
	initModes();
	this->clearBuffer();
}

Client::Client(int socket) : _socket(socket), _authorised(false), _registered(false), _terminate(false), _hostname_looked_up(false)
{
	memset(_buffer, 0, BUFFER_SIZE);
	_chan_limit = MAX_CLIENT_CHANNELS;
	_chan_count = 0;
	_last_activity = std::time(NULL);
	_pong_timer = std::time(NULL);
	_expected_pong = false;
	initModes();
	this->clearBuffer();
}

Client::~Client(void) {}

// Setters
void Client::setSocket(const int &socket)
{
	this->_socket = socket;
}

void Client::setSockaddr(const struct sockaddr *addr)
{
	this->_addr = addr;
}

void Client::setNickname(const std::string &nickname)
{
	this->_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	this->_username = username;
}

void Client::setHostname(const std::string &hostname)
{
	this->_hostname = hostname;
}

void Client::setRealname(const std::string &realname)
{
	this->_realname = realname;
}

void Client::setAuthorised(bool value)
{
	this->_authorised = value;
}

void Client::setRegistered(bool value)
{
	this->_registered = value;
}

void Client::setTerminate( bool value )
{
	this->_terminate = value;
}

void Client::setMode(char mode, bool value)
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

void Client::setLastActivity( void )
{
	this->_last_activity = std::time(NULL);
}

void Client::setPongTimer( void )
{
	this->_pong_timer = std::time(NULL);
}

void Client::setExpectedPong( bool value )
{
	this->_expected_pong = value;
}

void Client::setPassword( const std::string & password )
{
	this->_given_password = password;
}

// Getters
int Client::getSocket(void)
{
	return this->_socket;
}

const std::string &Client::getNickname(void) const
{
	return this->_nickname;
}

const std::string &Client::getUsername(void) const
{
	return this->_username;
}

const std::string Client::getHostname(void) const
{
	if (this->isRegistered() == false)
		return "0.0.0.0";
	return this->_hostname;
}

const std::string &Client::getRealname(void) const
{
	return this->_realname;
}

const std::string Client::getUserPrefix(void) const
{
	std::string	str = this->getNickname() + "!" + this->getUsername() + "@" + this->getHostname();

	return str;
}

bool Client::isAuthorised(void) const
{
	return this->_authorised;
}

bool Client::isRegistered(void) const
{
	return this->_registered;
}

bool Client::isTerminate( void ) const
{
	return this->_terminate;
}

bool Client::getMode(char mode) const
{
	switch (mode)
	{
		case 'a':
			return this->_mode.a;
			break ;
		case 'i':
			return this->_mode.i;
			break ;
		case 'w':
			return this->_mode.w;
			break ;
		case 'r':
			return this->_mode.r;
			break ;
		case 'o':
			return this->_mode.o;
			break ;
		case 'O':
			return this->_mode.O;
			break ;
		case 's':
			return this->_mode.s;
			break ;
		default:
			return false;
			break ;
	}
}

t_mode Client::getModes(void) const
{
	return this->_mode;
}

int Client::getChannelCount(void) const
{
	return this->_chan_count;
}

int Client::getChannelLimit(void) const
{
	return this->_chan_limit;
}

time_t Client::getLastActivity( void ) const
{
	return this->_last_activity;
}

time_t Client::getPongTimer( void ) const
{
	return this->_pong_timer;
}

bool Client::isExpectedPong( void ) const
{
	return this->_expected_pong;
}

bool Client::matchPassword( const std::string & match ) const
{
	return this->_given_password == match;
}

bool Client::passwordGiven( void ) const
{
	return !this->_given_password.empty();
}

// Channel Management
void Client::addChannel( Channel * channel, std::string & name )
{
	this->_channels.insert(std::pair<std::string, Channel *>(name, channel));
	this->_chan_count++;
}

void Client::removeChannel(std::string & name)
{
	_channels.erase(name);
	this->_chan_count--;
}

bool Client::hasMode(char mode) const
{
	return getMode(mode);
}

const std::string Client::getModeString(void) const
{
	std::string str("+");

	if (this->hasMode('a'))
		str += "a";
	if (this->hasMode('i'))
		str += "i";
	if (this->hasMode('w'))
		str += "w";
	if (this->hasMode('r'))
		str += "r";
	if (this->hasMode('o'))
		str += "o";
	if (this->hasMode('O'))
		str += "O";
	if (this->hasMode('s'))
		str += "s";

	return str;
}

std::vector<Channel *>	Client::getChannelsVector( void ) const
{
	std::vector<Channel *>	channels;
//	std::cout << "getChannelsVector" << std::endl;

//	std::cout << "this->_channels.size(): " << this->_channels.size() << std::endl;
	for (std::map<std::string, Channel *>::const_iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
	{
//		std::cout << "Channel name: " << it->first << std::endl;
//		std::cout << "channel->getName(): " << it->second->getName() << std::endl;
		channels.push_back(it->second);
	}
	return channels;
}

std::string Client::hostnameLookup( void )
{
	if (this->_addr->sa_family == AF_INET)
	{
		// IPv4
		struct sockaddr_in *addr_in = (struct sockaddr_in *)this->_addr;
		std::string ip_address = inet_ntoa(addr_in->sin_addr);

		// Get the hostname
		struct hostent *host_entry = gethostbyname(ip_address.c_str());
		if (host_entry && host_entry->h_name)
		{
			setHostname(std::string(host_entry->h_name));
			return std::string("*** Found your hostname, cached");
		}
		else
		{
			setHostname(ip_address);
			return std::string("*** Couldn't resolve your hostname; using your IP address instead");
		}
	}
	else if (this->_addr->sa_family == AF_INET6)
	{
		// IPv6
		setHostname("IPv6.42.fr");
		return std::string("*** IPv6 address detected; using a placeholder hostname");
	}
	// else, unknown family
	setHostname("Unknown.42.fr");
	return std::string("*** Unknown address family; couldn't resolve your hostname");
}

bool	Client::fillBuffer( std::string	src )
{
	std::string	str = this->_buffer;
	bool		flag = false;

	str += src;
	if (str.size() > BUFFER_SIZE - 3)
	{
		str.insert(1021, "\r\n");
		flag = true;
	}

	size_t	size = str.size();
	if (size > BUFFER_SIZE - 1)
		size = BUFFER_SIZE - 1;

	std::copy(str.c_str(), str.c_str() + size, _buffer);
	return flag;
}

void	Client::clearBuffer( void )
{
	std::fill(this->_buffer, _buffer + BUFFER_SIZE, 0);
}

std::string	Client::getBuffer( void ) const
{
	std::string str = this->_buffer;
	return str;
}
