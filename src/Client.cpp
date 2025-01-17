#include "Client.hpp"
#include <cstring>
#include <algorithm>

// Constructors and Destructor
Client::Client(void)
{
    memset(_buffer, 0, BUFFER_SIZE);
    _authorised = false;
    _registered = false;
    _terminate = false;
    _chan_limit = 0;
    _chan_count = -1;
}

Client::Client(int socket) : _socket(socket), _authorised(false), _registered(false), _chan_limit(0)
{
    memset(_buffer, 0, BUFFER_SIZE);
    _chan_count = -1;
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

const std::string &Client::getHostname(void) const
{
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
    // return static_cast<int>(this->_channels.size());
    return this->_chan_count;
}

int Client::getChannelLimit(void) const
{
    return this->_chan_limit;
}

// Channel Management
void Client::addChannel(Channel &channel, std::string &name)
{
    _channels[name] = &channel;
}

void Client::removeChannel(Channel &channel, std::string &name)
{
    _channels.erase(name);
	// TODO soomething with &channel??
	(void)channel;
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

	for (std::map<std::string, Channel *>::const_iterator it = this->_channels.begin(); it != this->_channels.end(); ++it)
		channels.push_back(it->second);
	return channels;
}

std::string Client::hostnameLookup( void )
{
    if (this->_addr->sa_family == AF_INET)
    {
        // IPv4: We can work with this
        struct sockaddr_in *addr_in = (struct sockaddr_in *)this->_addr;
        std::string ip_address = inet_ntoa(addr_in->sin_addr);

        // Get the hostname
        struct hostent *host_entry = gethostbyname(ip_address.c_str());
        if (host_entry && host_entry->h_name)
        {
            setHostname(std::string(host_entry->h_name));
            return std::string(":*** Found your hostname:" + this->_hostname);
        }
        else
        {
            setHostname(ip_address);
            return std::string(":*** Couldn't resolve your hostname; using your IP address instead");
        }
    }
    // else connection must be IPv6 (I think), TODO make sure
    // IPv6: Our allowed C functions are limted, we can't do much here
    setHostname("placeholder.42.fr");
    std::cout << this->_hostname << std::endl;
    return std::string(":*** Couldn't resolve your hostname; using a placeholder instead");
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
