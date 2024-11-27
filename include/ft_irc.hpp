#ifndef FT_IRC_HPP
#define FT_IRC_HPP

#include <iostream>
#include <cstdint>
#include <sstream>
#include <exception>

// C libraries for the functions specified in the subject:
#include <sys/socket.h>
/*
	socket
	setsockopt
	getsockname
	bind
	connect
	listen
	accept
	send
	recv
*/
#include <unistd.h>
/*
	close
	lseek
*/
#include <netdb.h>
/*
	getprotobyname
	gethostbyname
	getaddrinfo (sys/types.h, sys/socket.h ?)
	freeaddrinfo (sys/socket.h ?)
*/
#include <arpa/inet.h>
/*
	htons
	htonl
	ntohs
	ntohl
	inet_addr
	inet_ntoa
*/
#include <signal.h>
/*
	signal
	sigaction	
*/
#include <sys/stat.h>
/*
	fstat
*/
#include <fcntl.h>
/*
	fcntl (Only for macOS I think?)
*/
#include <poll.h>
/*
	poll (The subject says we can also use select, kqueue or epoll, among others possibly)
*/

typedef uint_fast16_t	t_port;

#endif // FT_IRC_HPP