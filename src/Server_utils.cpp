#include "ft_irc.hpp"
#include "Server.hpp"

bool Server::isUserInServer( const std::string & nickname ) 
{
	Client * client = findClient(nickname);
	if (client)
		return true;
	return false;
}

Client * Server::findClient( int fd ) 
{
	std::map<int, Client>::iterator it = this->_clients.find(fd);
	if (it == this->_clients.end())
		return (NULL); // apresas-: Maybe? I'd rather use references but I'm not sure how to handle this rn
	return &it->second;
}

Client * Server::findClient( const std::string & nickname ) 
{
	std::map<std::string, int>::iterator it = this->_clients_fd_map.find(nickname);
	if (it == this->_clients_fd_map.end())
		return (NULL);
	return findClient(it->second);
}

void Server::addChannel( Channel & channel, std::string & name )
{
    this->_channels.insert(std::pair<std::string, Channel>(name, channel));
}

bool Server::channelFound(const std::string& chanName)
{
    for (std::map<std::string, Channel>::const_iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->first == chanName)
            return true;
    }
    return false;
}

Channel * Server::channelGet( const std::string & name ) 
{
    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        if (it->first == name)
            return &it->second;
    }
	return (NULL);
}

bool Server::isChannelInServer( const std::string & name ) 
{
	Channel * channel = findChannel(name);
	if (channel)
		return true;
	return false;
}

Channel * Server::findChannel( const std::string & name ) 
{
	std::map<std::string, Channel>::iterator it = this->_channels.find(name);
	if (it == this->_channels.end())
		return (NULL);
	return &it->second;
}

bool	Server::hasNULL( const char * buffer, int bytes_received ) const
{
	for (int i = 0; i < bytes_received; i++)
		if (buffer[i] == '\0')
			return true;
	return false;
}

bool	Server::hasCRLF( const std::string str ) const
{
	if (str.size() > 1)
		return str[str.size() - 2] == '\r' && str[str.size() - 1] == '\n';
	return false;
}

std::string	Server::stringToUpper( std::string src )
{
	std::string str;
	for (std::string::iterator it = src.begin(); it != src.end(); it++)
		str += toupper(*it);
	return str;
}

void	Server::printTmessage( t_message message ) const 
{
	std::cout << "t_message: ";
	std::cout << "Prefix [" << message.prefix << "] ";
	std::cout << "Command [" << message.command << "] ";
	std::cout << "Params ";
	for (size_t i = 0; i < message.params.size(); i++)
		std::cout << "[" << message.params[i] << "] ";
	std::cout << std::endl;
	std::cout << "Sender: [ " << message.sender_client_fd << " ]" << std::endl;
	std::cout << "Targets: ";
	for (std::set<int>::iterator it = message.target_client_fds.begin(); it != message.target_client_fds.end(); it++)
	{
		std::cout << "[ " << *it << " ]";
	}
	std::cout << std::endl;
	std::cout << std::endl;
}

void	Server::addChannelToReply( t_message & reply, Channel * channel )
{
	std::map<std::string, Client*>	users = channel->getTrueUsers();
	for ( std::map<std::string, Client*>::iterator it = users.begin(); it != users.end(); it++ )
	{
		reply.target_client_fds.insert(it->second->getSocket());
	}
}

void	Server::addChannelToReplyExcept( t_message & reply, Channel * channel )
{
	std::map<std::string, Client*>	users = channel->getTrueUsers();
	for ( std::map<std::string, Client*>::iterator it = users.begin(); it != users.end(); it++ )
	{
		if (it->second->getSocket() == this->_current_client->getSocket())
			continue ;
		reply.target_client_fds.insert(it->second->getSocket());
	}
}

void	Server::addUserToChannel( std::string channel_name, Client * client, bool as_operator )
{
	Channel * channel = findChannel(channel_name);
	if (channel)
	{
		channel->addUser(client, as_operator);
		client->addChannel(channel, channel_name);
	}
}

bool	Server::isNicknameTaken( const std::string & nickname ) const
{
	std::vector<std::string>::const_iterator it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), nickname);
	return (it != this->_taken_nicknames.end());
}

void	Server::removeTakenNickname( const std::string & nickname )
{
	std::vector<std::string>::iterator it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), nickname);
	if (it != this->_taken_nicknames.end())
		this->_taken_nicknames.erase(it);
}

void	Server::replaceTakenNickname( Client * client, const std::string & new_nickname )
{
	std::string old_nickname = client->getNickname();
	std::vector<std::string>::iterator it = std::find(this->_taken_nicknames.begin(), this->_taken_nicknames.end(), old_nickname);
	if (it != this->_taken_nicknames.end())
		*it = new_nickname;
	else
		this->_taken_nicknames.push_back(new_nickname);
}

void	Server::updateClientNickname( Client * client, const std::string & new_nickname )
{
	std::string old_nickname = client->getNickname();
	if (new_nickname.empty() == true)
		return ;
	if (old_nickname == new_nickname)
		return ;
	this->_clients_fd_map.erase(old_nickname);
	this->_clients_fd_map.insert(std::pair<std::string, int>(new_nickname, client->getSocket()));
	this->replaceTakenNickname(client, new_nickname);
	client->setNickname(new_nickname);
}

t_message	Server::createNotice( Client * client, const std::string & message )
{
	t_message notice;
	std::string nickname;
	notice.prefix = ":" + this->getName();
	notice.command = "NOTICE";
	notice.target_client_fds.insert(client->getSocket());
	nickname = client->getNickname();
	if (nickname.empty())
		nickname = "*";
	notice.params.push_back(nickname);
	notice.params.push_back(":" + message);
	return notice;
}

void Server::removeChannel(const std::string &name)
{
    std::map<std::string, Channel>::iterator it = this->_channels.find(name);
    if (it == this->_channels.end())
    {
        std::cerr << "Error: Channel \"" << name << "\" not found." << std::endl;
        return;
    }
    this->_channels.erase(it);
    std::cout << "Channel \"" << name << "\" has been removed successfully." << std::endl;
}
