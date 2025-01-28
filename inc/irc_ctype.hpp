#ifndef IRC_CTYPE_HPP
#define IRC_CTYPE_HPP

#include <cctype>
#include <cstring>
#include <string>

#define SPECIAL_CHARACTERS "[\\]`_^{|}"

#define MAX_CHANNEL_NAME_LENGTH 50
#define CHANNEL_PREFIXES "#"
#define CHANNEL_NAME_FORBIDDEN_CHARS "\0\6\r\n ,:"

bool	isNospcrlfcl( char c );
bool	isHexdigit( char c );
bool	isUpper( char c );
bool	isLower( char c );
bool	isSpecial( char c );

bool	isValidNickname( const std::string & nickname );
bool	isValidChannelName( const std::string & channel_name ); // Move this maybe

#endif //IRC_CTYPE_HPP

