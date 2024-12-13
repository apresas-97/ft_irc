#include <irc_ctype.hpp>

/* Some syntax semantics for IRC, in BNF:

	message = [ ":" prefix SPACE ] command [ params ] crlf
	prefix     =  servername / ( nickname [ [ "!" user ] "@" host ] )
	command    =  1*letter / 3digit
	params	= *14( SPACE middle ) [ SPACE ":" trailing ]
			/= 14( SPACE middle ) [ SPACE [":"] trailing ]
	letter = a-z | A-Z
	digit = 0-9
	hexdigit digit | A-F
	special = "[" / "]" / "\" / "`" / "_" / "^" / "{" / "|" / "}"
	
	nospcrlfcl = Any octet except NUL, CR, LF, " " and ":"

	middle = nospcrlfcl *( ":" / nospcrlfcl )
	
	trailing = *( ":" / ' ' / nospcrlfcl )
	
	SPACE = ' '

	crlf = "\r\n"
*/

/* nospcrlfcl = Any octet except '\0', \r', '\n', ' ', and ':' */
bool	irc_isNospcrlfcl( char c ) 
{
	return (c != '\0' && c != '\r' && c != '\n' && c != ' ' && c != ':');
}

// Equivalent to isalpha()
bool	irc_isLetter( char c )
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

// Equivalent to isdigit()
bool	irc_isDigit( char c )
{
	return (c >= '0' && c <= '9');
}

// hexdigit = 0-9 || A-F
bool	irc_isHexdigit( char c )
{
	return (irc_isDigit(c) || (c >= 'A' && c <= 'F'));
}

// special = '[' - '_' || '{' - '}'
bool	irc_isSpecial( char c ) 
{
	return (c >= 91 && c <= 96) || (c >= 123 && c <= 125);
}

// uppercase = A-Z || "[]\~"
bool	irc_isUpper( char c ) 
{
	return (c >= 'A' && c <= ']' || c == '~');
}

// lowercase = a-z || "{}|^"
bool	irc_islower( char c ) 
{
	return (c >= 'a' && c <= '}');
}

/*
	key = 1*23( %x01-05 / %x07-08 / %x0C / %x0E-1F / %x21-7F )
		; A series of 1 to 23 octets
		; can contain any 7-bit ASCII character, except NUL, ACK, HT, LF, VT, FF, CR, or SPACE 
*/
// TODO: This function should probably be part of the Client class
bool	isKeyValid( const std::string & key )
{
	// KEY_MIN_LEN = 1
	// KEY_MAX_LEN = 23
	if (key.size() < 1 || key.size() > 23)
		return false;
	for (size_t i = 0; i < key.size(); i++) 
	{
		if (key[i] == '\0' || key[i] == '\6' || key[i] == '\t' || key[i] == '\n' || key[i] == '\v' || key[i] == '\f' || key[i] == '\r' || key[i] == ' ')
			return false;
	}
	return true;
}

// TODO: This function should probably be part of the Channel class
bool	isChannelNameValid( const std::string & channel_name )
{
	if (channel_name.size() < 1 || channel_name.size() > 50) // Use MAX CHANNEL NAME LENGTH macro here when this is integrated into the channel class
		return false;
	if (!irc_isChanPrefix(channel_name[0]))
		return false;
	for (size_t i = 0; i < channel_name.size(); i++) 
	{
		if (channel_name[i] == '\0' || channel_name[i] == '\6' || channel_name[i] == '\r' || channel_name[i] == '\n' || channel_name[i] == ' ' || channel_name[i] == ',' || channel_name[i] == ':')
			return false;
	}
	return true;
}

// TODO: This function should probably be part of the Channel class
bool	irc_isChanPrefix( char c )
{
	return (c == '#' || c == '&' || c == '+');
}

// TODO: This function should probably be part of the Client class
bool	irc_isValidNickname( const std::string & nickname ) 
{
	if (nickname.size() < 1 || nickname.size() > 9)
		return false;
	if (!irc_isLetter(nickname[0]) && !irc_isSpecial(nickname[0]))
		return false;
	for (size_t i = 1; i < nickname.size(); i++) 
	{
		if (!irc_isLetter(nickname[i]) && !irc_isDigit(nickname[i]) && !irc_isspecial(nickname[i]) && nickname[i] != '-')
			return false;
	}
	return true;
}
