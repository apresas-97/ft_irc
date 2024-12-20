#ifndef REPLY_CODES_HPP
#define REPLY_CODES_HPP

// RPL_:
// Welcome messages
#define RPL_WELCOME 1
#define RPL_WELCOME_STR "Welcome to the Internet Relay Network <identifier(nickname!user@host)>"
#define RPL_YOURHOST 2
#define RPL_YOURHOST_STR "Your host is <servername>, running version <ver>"
#define RPL_CREATED 3
#define RPL_CREATED_STR "This server was created <date>"
#define RPL_MYINFO 4
#define RPL_MYINFO_STR "<servername> <version> <available user modes> <available channel modes>"
#define RPL_PASSWORD 900
#define RPL_PASSWORD_STR "<nickname> :Please provide the password to continue"

// If server is full
#define RPL_BOUNCE 5
#define RPL_BOUNCE_STR "Try server <server name>, port <port number>"

#define RPL_UMODEIS 221
#define RPL_UMODEIS_STR "<user_modes>"

#define RPL_CHANNELMODEIS 324
#define RPL_CHANNELMODEIS_STR "<channel> <mode> <mode_params>"

// ERR_:
#define ERR_NOSUCHCHANNEL 403
#define ERR_NOSUCHCHANNEL_STR "<channel> :No such channel"
#define ERR_UNKNOWNCOMMAND 421
#define ERR_UNKNOWNCOMMAND_STR "<command> :Unknown command"
#define ERR_NONICKNAMEGIVEN 431
#define ERR_NONICKNAMEGIVEN_STR ":No nickname given"
#define ERR_ERRONEUSNICKNAME 432
#define ERR_ERRONEUSNICKNAME_STR "<nickname> :Erroneous nickname"
#define ERR_NICKNAMEINUSE 433
#define ERR_NICKNAMEINUSE_STR "<nickname> :Nickname is already in use"
#define ERR_UNAVAILRESOURCE 437
#define ERR_UNAVAILRESOURCE_STR "<nickname/channel> :Nick/channel is temporarily unavailable"
#define ERR_NEEDMOREPARAMS 461
#define ERR_NEEDMOREPARAMS_STR "<nickname> :Not enough parameters"
#define ERR_ALREADYREGISTRED 462
#define ERR_ALREADYREGISTRED_STR ":Unauthorized command (already registered)"
#define ERR_PASSWDMISMATCH 464
#define ERR_PASSWDMISMATCH_STR ":Password incorrect"
#define ERR_UNKNOWNMODE 472
#define ERR_UNKNOWNMODE_STR "<char> :is unknown mode char to me for <channel>"
#define ERR_NOCHANMODES 477
#define ERR_NOCHANMODES_STR "<channel> :Channel doesn't support modes"
#define ERR_RESTRICTED 484
#define ERR_RESTRICTED_STR ":Your connection is restricted!"
#define ERR_UMODEUNKNOWNFLAG 501
#define ERR_UMODEUNKNOWNFLAG_STR ":Unknown MODE flag"
#define ERR_USERSDONTMATCH 502
#define ERR_USERSDONTMATCH_STR ":Cannot change mode for other users"

#endif // REPLY_CODES_HPP

