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
bool	isNospcrlfcl( char c )
{
	return (c != '\0' && c != '\r' && c != '\n' && c != ' ' && c != ':');
}

// hexdigit = 0-9 || A-F
bool	isHexdigit( char c )
{
	return (isdigit(c) || (c >= 'A' && c <= 'F'));
}

// special = '[' - '_' || '{' - '}'
bool	isSpecial( char c )
{
	return (c >= 91 && c <= 96) || (c >= 123 && c <= 125);
}

// uppercase = A-Z || "[]\~"
bool	isUpper( char c )
{
	return ((c >= 'A' && c <= ']') || c == '~');
}

// lowercase = a-z || "{}|^"
bool	isLower( char c )
{
	return (c >= 'a' && c <= '}');
}

/*
	key = 1*23( %x01-05 / %x07-08 / %x0C / %x0E-1F / %x21-7F )
		; A series of 1 to 23 octets
		; can contain any 7-bit ASCII character, except NUL, ACK, HT, LF, VT, FF, CR, or SPACE 
*/
bool	isKeyValid( const std::string & key )
{
	if (key.size() < 1 || key.size() > 23)
		return false;
	for (size_t i = 0; i < key.size(); i++) 
	{
		if (key[i] == '\0' || key[i] == '\6' || key[i] == '\t' || key[i] == '\n' || key[i] == '\v' || key[i] == '\f' || key[i] == '\r' || key[i] == ' ')
			return false;
	}
	return true;
}

bool	isValidNickname( const std::string & nickname )
{
	if (nickname.size() < 1 || nickname.size() > 9)
		return false;
	if (!isalpha(nickname[0]) && !isSpecial(nickname[0]))
		return false;
	for (size_t i = 1; i < nickname.size(); i++)
	{
		if (!isalnum(nickname[i]) && nickname[i] != '_' && nickname[i] != '-')
			return false;
	}
	return true;
}

bool	isValidChannelName( const std::string & channel_name )
{
	if (channel_name.size() < 1 || channel_name.size() > MAX_CHANNEL_NAME_LENGTH)
		return false;
	if (std::strchr(CHANNEL_PREFIXES, channel_name[0]) == NULL)
		return false;
	for (size_t i = 0; i < channel_name.size(); i++)
	{
		if (std::strchr(CHANNEL_NAME_FORBIDDEN_CHARS, channel_name[i]) != NULL)
			return false;
	}
	return true;
}
