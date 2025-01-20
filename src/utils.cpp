#include "utils.hpp"

std::string uLongToString( size_t number )
{
	std::ostringstream oss;
	oss << number;
	return oss.str();
}
