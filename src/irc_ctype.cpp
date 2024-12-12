#include <irc_ctype.hpp>

bool	irc_isspecial( char c ) {
	if (c >= 91 && c <= 95)
		return true;
	if (c >= 123 && c <= 125)
		return true;
	return false;
}

bool	irc_isupper( char c ) {
	if (c >= 'A' && c <= ']')
		return true;
	return false;
}

bool	irc_islower( char c ) {
	if (c >= 'a' && c <= '}')
		return true;
	return false;
}

bool	irc_isValidNickname( const std::string & nickname ) {
	if (nickname.size() > 9 || nickname.empty())
		return false;
	return true;
}

bool	irc_isValidNickname( const std::string & nickname ) {
	if (nickname.size() > 9)
		return false;
	if (!isalpha(nickname[0] && !irc_isspecial(nickname[0])))
		return false;
	for (size_t i = 1; i < nickname.size(); i++) {
		if (!isalnum(nickname[i]) && !irc_isspecial(nickname[i]) && nickname[i] != '-')
			return false;
	}
	return true;
}
