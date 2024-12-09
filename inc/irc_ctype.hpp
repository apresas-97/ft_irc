#ifndef IRC_CTYPE_HPP
#define IRC_CTYPE_HPP

#include <cctype>
#include <string>

bool	irc_isspecial( char c );
bool	irc_isupper( char c );
bool	irc_islower( char c );

bool	irc_isValidNickname( const std::string & nickname );

#endif IRC_CTYPE_HPP