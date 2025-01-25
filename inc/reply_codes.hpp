#ifndef REPLY_CODES_HPP
#define REPLY_CODES_HPP

// RPL_:
// Welcome messages
#define RPL_WELCOME 1
#define RPL_WELCOME_STR ":Welcome to the Internet Relay Network <identifier(nickname!user@host)>"
#define RPL_YOURHOST 2
#define RPL_YOURHOST_STR ":Your host is <servername>, running version <ver>"
#define RPL_CREATED 3
#define RPL_CREATED_STR ":This server was created <date>"
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
#define RPL_CHANNELMODEIS_STR "<channel> +<mode> <mode_params>"

#define RPL_INVITING 341
#define RPL_INVITING_STR "<channel> <nick>"

#define RPL_TOPIC 332
#define RPL_TOPIC_STR "<channel> :Topic is <topic>"

#define RPL_NOTOPIC 331
#define RPL_NOTOPIC_STR "<channel> :No topic is set"

#define RPL_VERSION 351
#define RPL_VERSION_STR "<version>.<debuglevel> <server> :<comments>"

#define RPL_NAMREPLY 353
#define RPL_NAMREPLY_STR "= <channel> :<nick1> <nick2> <nick3> ..."

#define RPL_TIME 391
#define RPL_TIME_STR "<server> :<string showing server's local time>"

#define RPL_ENDOFNAMES 366
#define RPL_ENDOFNAMES_STR "<channel> :End of /NAMES list"

#define RPL_MOTDSTART 375
#define RPL_MOTDSTART_STR ":- <server> Message of the day - "

#define RPL_MOTD 372
#define RPL_MOTD_STR ":- <text>"

#define RPL_ENDOFMOTD 376
#define RPL_ENDOFMOTD_STR ":End of MOTD command"

// Channel removed
#define RPL_CHANNELREMOVED 901
#define RPL_CHANNELREMOVED_STR "<channel> :Channel has been removed due to no active users"

// ERR_:
#define ERR_NOSUCHSERVER 402
#define ERR_NOSUCHSERVER_STR "<server name> :No such server"
#define ERR_NOSUCHCHANNEL 403
#define ERR_NOSUCHCHANNEL_STR "<channel> :No such channel"
#define ERR_NOORIGIN 409
#define ERR_NOORIGIN_STR ":No origin specified"
#define ERR_UNKNOWNCOMMAND 421
#define ERR_UNKNOWNCOMMAND_STR "<command> :Unknown command"
#define ERR_NOMOTD 422
#define ERR_NOMOTD_STR ":MOTD File is missing"
#define ERR_NONICKNAMEGIVEN 431
#define ERR_NONICKNAMEGIVEN_STR ":No nickname given"
#define ERR_ERRONEUSNICKNAME 432
#define ERR_ERRONEUSNICKNAME_STR "<nickname> :Erroneous nickname"
#define ERR_NICKNAMEINUSE 433
#define ERR_NICKNAMEINUSE_STR "<nickname> :Nickname is already in use"
#define ERR_UNAVAILRESOURCE 437
#define ERR_UNAVAILRESOURCE_STR "<nickname/channel> :Nick/channel is temporarily unavailable"
#define ERR_USERNOTINCHANNEL 441
#define ERR_USERNOTINCHANNEL_STR "<nick> <channel> :They aren't on that channel"
#define ERR_NOTREGISTERED 451
#define ERR_NOTREGISTERED_STR ":You have not registered"
#define ERR_NEEDMOREPARAMS 461
#define ERR_NEEDMOREPARAMS_STR "<command> :Not enough parameters"
#define ERR_ALREADYREGISTRED 462
#define ERR_ALREADYREGISTRED_STR ":Unauthorized command (already registered)"
#define ERR_PASSWDMISMATCH 464
#define ERR_PASSWDMISMATCH_STR ":Password incorrect"
#define ERR_KEYSET 467
#define ERR_KEYSET_STR "<channel> :Channel key already set"
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
#define ERR_NOSUCHCHANNEL 403
#define ERR_NOSUCHCHANNEL_STR "<channel> :No such channel"
#define ERR_UNKNOWNCOMMAND 421
#define ERR_UNKNOWNCOMMAND_STR "<command> :Unknown command"
#define ERR_NOTONCHANNEL 442
#define ERR_NOTONCHANNEL_STR "<channel> :You're not on that channel"
#define ERR_USERONCHANNEL 443
#define ERR_USERONCHANNEL_STR "<nick> <channel> :is already on channel"
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_CHANOPRIVSNEEDED_STR "<channel> :You're not channel operator"
#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHNICK_STR "<nick> :No such nick/channel"
#define ERR_INVITEONLYCHAN 473
#define ERR_INVITEONLYCHAN_STR "<channel> :Cannot join channel (+i)"
#define ERR_CHANNELISFULL 471
#define ERR_CHANNELISFULL_STR "<channel> :Cannot join channel (+l)"
#define ERR_TOOMANYCHANNELS 405
#define ERR_TOOMANYCHANNELS_STR "<channel> :You have joined too many channels"
#define ERR_BADCHANNELKEY 475
#define ERR_BADCHANNELKEY_STR "<channel> :Cannot join channel (+k)"
#define ERR_BADCHANMASK 476
#define ERR_BADCHANMASK_STR "<channel> :Bad Channel Mask"
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_CANNOTSENDTOCHAN_STR "<channel> :Cannot send to channel"
#define ERR_NORECIPIENT 411
#define ERR_NORECIPIENT_STR ":No recipient given (<command>)"
#define ERR_NOTEXTTOSEND 412
#define ERR_NOTEXTTOSEND_STR ":No text to send"

#endif // REPLY_CODES_HPP
