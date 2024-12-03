#include "ft_irc.hpp"
#include "Server.hpp"

/*
	Recommended IRC server ports:
		6667 - For non-encrypted connections
		6697 - For TLS-encrypted connections
*/
// t_port	parse_port( const char *cstr_port )
// {
// 	t_port port;
// 	std::string port_str(cstr_port);
// 	std::istringstream iss(port_str);

// 	iss >> port;
// 	if (port_str.empty() || iss.fail() || !iss.eof())
// 		throw std::invalid_argument("Invalid port input received");
// 	else if (port < 1024 || port > 49151)
// 		throw std::invalid_argument("Port must be between 1024 and 49151");
// 	return port;
// }

// std::string	parse_password( const char *cstr_password )
// {
// 	std::string password(cstr_password);
// 	// Do we need to check anything?
// }

int	main( int argc, char **argv ) // Maybe envp?
{
	if (argc != 3)
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	// std::string password;
	// t_port port;
	try
	{
		// AA: esto podriamos ponerlo en un initServ dentro del constructor, mandando ahi port y password
		Server	server(argv[1], argv[2]);
		// server.run();
		// port = parse_port(argv[1]);
		// password = parse_password(argv[2]);
	}
	catch ( std::invalid_argument & e )
	{
		std::cerr << e.what() << " => \"" << argv[1] << "\"" << std::endl;
	}
	// Init Server object with port and password, will be references so they must be set on init

	return 0;
}
