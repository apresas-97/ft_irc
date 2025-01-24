#include "Server.hpp"


// NOTE: The IRC protocol doesn't specify syntax rules for the username and realname
static bool	irc_isValidUsername( const std::string & username )
{
	if (username.size() < 1 || username.size() > 15)
		return false;
	for (size_t i = 0; i < username.size(); i++)
	{
		// Only allow printable ASCII characters, except spaces
		if (username[i] < 33 || username[i] > 126)
			return false;
	}
	return true;
}

// NOTE: The IRC protocol doesn't specify syntax rules for the username and realname
static bool	irc_isValidRealname( const std::string & realname )
{
	if (realname.size() < 1 || realname.size() > 50)
		return false;
	for (size_t i = 0; i < realname.size(); i++)
	{
		// Only allow printable ASCII characters
		if (realname[i] < 32 || realname[i] > 126)
			return false;
	}
	return true;
}

std::vector<t_message>	Server::createWelcomeReplies( Client * client )
{
	std::vector<t_message>	welcome_replies;

	welcome_replies.push_back(createReply(RPL_WELCOME, RPL_WELCOME_STR, client->getUserPrefix()));

	std::vector<std::string> yourhost_params;
	yourhost_params.push_back(this->getName());
	yourhost_params.push_back(this->getVersion());

	welcome_replies.push_back(createReply(RPL_YOURHOST, RPL_YOURHOST_STR, yourhost_params));

	welcome_replies.push_back(createReply(RPL_CREATED, RPL_CREATED_STR, this->getStartTimeStr()));

	std::vector<std::string> myinfo_params;
	myinfo_params.push_back(this->getName());
	myinfo_params.push_back(this->getVersion());
	// These are TODO because I don't know if those are the definitive modes or if this is how we'll handle it
	myinfo_params.push_back(USER_MODES); // TODO: Here will go all of the available user modes
	myinfo_params.push_back(CHANNEL_MODES); // TODO: Here will go all of the available channel modes

	welcome_replies.push_back(createReply(RPL_MYINFO, RPL_MYINFO_STR, myinfo_params));

	return welcome_replies;
}

/*
Command: USER
Parameters: <username> <unused> <unused> <realname>
USER command is used at the beginning of connection to specify the identity of the user.
There are two legacy parameters which are unused nowadays.

// OLD DEFINITION:
Parameters: <username> <mode> <unused> <realname>
USER command is used at the beginning of connection to specify the identity of the user.
<mode> should be numeric, and it's a bitmask for modes 'w'(bit 2) and 'i'(bit 3). The rest of bits
are irrelevant.
<realname> may contain space characters.
*/
std::vector<t_message>	Server::cmdUser( t_message & message ) 
{
	std::cout << "USER command called..." << std::endl;
	Client *	client = this->_current_client;
	std::vector<t_message>	replies;

	// Check if the client is already registered, this takes precedence over everything else
	if (client->isRegistered() == true)
	{
		replies.push_back(createReply(ERR_ALREADYREGISTRED, ERR_ALREADYREGISTRED_STR));
		return replies;
	}

	if (message.params.size() < 4) 
	{
		replies.push_back(createReply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, this->_current_client->getNickname()));
		return replies;
	}

	if (this->_current_client->getNickname().empty() == true) 
	{
		/* I haven't been able to get a definitive answer on the order of the NICK and USER commands
		But from what I've gathered, it really seems like the NICK command should come first
		For this reason, if the USER message is sent first, the server will silently ignore it */
		return replies;
	}

	// NOTICE reply for invalid username or realname syntax
	t_message notice_reply;
	notice_reply.prefix = ":" + this->getName();
	notice_reply.command = "NOTICE";
	notice_reply.target_client_fds.insert(client->getSocket());

	if (irc_isValidUsername(message.params[0]) == false)
	{
		notice_reply.params.push_back(client->getNickname() + " :Invalid username syntax (only printable ASCII and must be between 1 and 15 characters)");
		replies.push_back(notice_reply);
		return replies;
	}
	if (irc_isValidRealname(message.params[3]) == false)
	{
		notice_reply.params.push_back(client->getNickname() + " :Invalid realname syntax (only printable ASCII and must be between 1 and 50 characters)");
		replies.push_back(notice_reply);
		return replies;
	}

	client->setUsername(message.params[0]);
	client->setRealname(message.params[3]);

	// If the client is not authorised, then of course, it won't be registered and won't get the WELCOME replies
	if (client->isAuthorised() == false)
		return replies;

	client->setRegistered(true);

	return createWelcomeReplies(client);
}

