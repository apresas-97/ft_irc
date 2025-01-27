#include <ctime>
#include <iomanip>

#include "Server.hpp"

Server*	Server::instance = NULL;

Server::Server( const std::string & port, const std::string & password ) : _port(port), _password(password) 
{
	instance = this;
	setName("ft_irc.42.fr");
	setVersion(1, 0);
	setStartTime();
	parseInput();
	this->_client_count = 0;
	initServer();
}

Server::~Server( void ) 
{
	cleanClose(false);
}

void Server::signalHandler( int signal ) 
{
	if (signal == SIGINT) 
	{
		if (instance)
			instance->cleanClose(true);
		exit(EXIT_SUCCESS);
	}
}

void Server::sendData(const char *message) 
{
	for (size_t i = 1; i < _poll_fds.size(); i++) 
		if (_poll_fds[i].fd != -1) 
			send(_poll_fds[i].fd, message, strlen(message), 0);
}

void Server::setName( const std::string & name )
{
	this->_name = name;
}

std::string Server::getName( void ) const 
{
	return this->_name;
}

void Server::setVersion( size_t major, size_t minor ) 
{
	this->_version_major = major;
	this->_version_minor = minor;
}

std::string Server::getVersion( void ) const 
{
	std::ostringstream oss;

	oss << this->_version_major;
	oss << ".";
	oss << this->_version_minor;
	std::string version = oss.str();
	return version;
}

std::string Server::getTimestamp( time_t time )
{
	std::tm *now = std::localtime(&time);

	std::ostringstream oss;
	switch (now->tm_wday) 
	{
		case 0:
			oss << "Sun, ";
			break;
		case 1:
			oss << "Mon, ";
			break;
		case 2:
			oss << "Tue, ";
			break;
		case 3:
			oss << "Wed, ";
			break;
		case 4:
			oss << "Thu, ";
			break;
		case 5:
			oss << "Fri, ";
			break;
		case 6:
			oss << "Sat, ";
			break;
	}
	switch (now->tm_mon) 
	{
		case 0:
			oss << "Jan ";
			break;
		case 1:
			oss << "Feb ";
			break;
		case 2:
			oss << "Mar ";
			break;
		case 3:
			oss << "Apr ";
			break;
		case 4:
			oss << "May ";
			break;
		case 5:
			oss << "Jun ";
			break;
		case 6:
			oss << "Jul ";
			break;
		case 7:
			oss << "Aug ";
			break;
		case 8:
			oss << "Sep ";
			break;
		case 9:
			oss << "Oct ";
			break;
		case 10:
			oss << "Nov ";
			break;
		case 11:
			oss << "Dec ";
			break;
	}
	oss << std::setw(2) << std::setfill('0') << now->tm_mday << " ";
	oss << std::setw(4) << std::setfill('0') << now->tm_year + 1900 << " at ";
	oss << std::setw(2) << std::setfill('0') << now->tm_hour + 1 << ":";
	oss << std::setw(2) << std::setfill('0') << now->tm_min << ":";
	oss << std::setw(2) << std::setfill('0') << now->tm_sec << " UTC";

	return oss.str();
}

void Server::setStartTime( void )
{
	this->_start_time = std::time(0);
	this->_start_time_str = getTimestamp(this->_start_time);
}

std::string Server::getStartTimeStr( void ) 
{
	return this->_start_time_str;
}

void Server::cleanClose( bool flag ) 
{
	for (std::vector<struct pollfd>::iterator it = _poll_fds.begin(); it != _poll_fds.end(); ++it)
	{
		if (close((*it).fd) == -1)
		{
			if ((*it).fd != this->_serverFd)
				closeFailureLog("_poll_fds", (*it).fd, this->_serverFd);
			else if ((*it).fd == this->_serverFd)
				closeFailureLog("serverFd", this->_serverFd);
		}
	}
	_poll_fds.erase(_poll_fds.begin(), _poll_fds.end());
	if (flag)
		this->~Server();
}

