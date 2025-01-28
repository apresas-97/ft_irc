#include "ft_irc.hpp"
#include "Server.hpp"

int	main( int argc, char **argv ) // Maybe envp?
{
	if (argc != 3) 
	{
		std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
		return 1;
	}
	try 
	{
		Server	server(argv[1], argv[2]);
	}
	catch ( std::exception & e ) 
	{
		std::cerr << "Error: " << e.what() << std::endl;
	}

	return 0;
}
