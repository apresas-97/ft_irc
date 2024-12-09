#include "Client.hpp"

Client::Client( int fd, struct sockaddr_storage address ) : _registered(false), _authorised(false) {
	this->_address = address;
}

Client::~Client( void ) {
	// ?
}
