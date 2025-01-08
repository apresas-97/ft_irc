#include "Server.hpp"

/*
Command: USER
Parameters: <username> <mode> <unused> <realname>
USER command is used at the beginning of connection to specify the identity of the user.
<mode> should be numeric, and it's a bitmask for modes 'w'(bit 2) and 'i'(bit 3). The rest of bits
are irrelevant.
<realname> may contain space characters.

New info:
Apparently, nowadays, the <mode> parameter is ignored completely by most servers.
*/
std::vector<t_message>	Server::cmdUser( t_message & message ) 
{
	std::cout << "USER command called..." << std::endl;
	std::vector<t_message>	replies;
	t_message				reply;
	if (message.params.size() < 4) 
	{
		reply = createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, this->_current_client->getNickname());
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	if (this->_current_client->getNickname().empty() == true) 
	{
		// Idk what to do here, the nickname should have been set already
		// If not, the registration can't continue
		// I need to figure this out
//		return ; // ??? // Can't return it's a std::vector<t_message>
	}
	Client * client = this->_current_client;
	if (client->isRegistered() == true) 
	{
		reply = createReply(ERR_ALREADYREGISTRED, ERR_ALREADYREGISTRED_STR);
		reply.target_client_fd = message.sender_client_fd;
		reply.sender_client_fd = _serverFd;
		replies.push_back(reply);
		return replies;
	}
	// apresas-: TODO: Check that the username and realname are not too long
	// But if they are, I don't know how to handle it yet because the protocol doesn't
	// specify what to do in that case
	// It doesn't really even specify a limit for username or realname length
	// But we should set some reasonable limit or we could be vulnerable to
	// all sort of buffer overflow attacks, etc.
	client->setUsername(message.params[0]);
	client->setHostname(message.params[1]);
	client->setRealname(message.params[3]);

	client->setRegistered(true);

	// TODO:
	// Prepare the welcome message reply
	reply = createReply(RPL_WELCOME, RPL_WELCOME_STR, client->getUserPrefix());
	reply.target_client_fd = message.sender_client_fd;
	reply.sender_client_fd = _serverFd;
	replies.push_back(reply);

	std::vector<std::string> yourhost_params;
	yourhost_params.push_back(this->getName());
	yourhost_params.push_back(this->getVersion());

	reply = createReply(RPL_YOURHOST, RPL_YOURHOST_STR, yourhost_params);
	reply.target_client_fd = message.sender_client_fd;
	reply.sender_client_fd = _serverFd;	
	replies.push_back(reply);

	reply = createReply(RPL_CREATED, RPL_CREATED_STR, this->getStartTimeStr());
	reply.target_client_fd = message.sender_client_fd;
	reply.sender_client_fd = _serverFd;		
	replies.push_back(reply);

	std::vector<std::string> myinfo_params;
	myinfo_params.push_back(this->getName());
	myinfo_params.push_back(this->getVersion());
	myinfo_params.push_back(USER_MODES); // Here will go all of the available user modes
	myinfo_params.push_back(CHANNEL_MODES); // Here will go all of the available channel modes

	reply = createReply(RPL_MYINFO, RPL_MYINFO_STR, myinfo_params);
	reply.target_client_fd = message.sender_client_fd;
	reply.sender_client_fd = _serverFd;	
	replies.push_back(reply);

	return replies;
}
