#include <iostream>
#include <vector>
#include <cassert>
#include <string>
#include <map>
#include <sstream>
#include <algorithm>
#include <ctime>
#include <iomanip>

#include "inc/reply_codes.hpp"

/*
This file tests the MODE command and its responses
The most advanced version of the MODE command is this one, so don't delete this file yet
*/

#define USER_MODES "aiwroOs"
#define CHANNEL_MODES "biklmnopstv"

std::vector<std::string> channels = { "#lobby", "#general", "#random", "#help"};
std::vector<std::string> nicks_in_channel = { "aaespino", "apresas", "ffornes", "miscoJon" };
std::string server_name = "irc_server";
std::string client_nickname = "apresas";

typedef struct s_message 
{
	std::string prefix;
	std::string command;
	std::vector<std::string> params;
	int sender_fd;
	int receiver_fd;
}				t_message;

typedef struct s_modes
{
	bool	a; // AWAY 
	bool	i; // INVISIBLE
	bool	w; // WALLOPS
	bool	r; // RESTRICTED
	bool	o; // OPERATOR
	bool	O; // LOCAL OPERATOR
	bool	s; // RECEIVE SERVER NOTICES
	bool	getMode( char mode ) const
	{
		switch (mode)
		{
			case 'a':
				return this->a;
			case 'i':
				return this->i;
			case 'w':
				return this->w;
			case 'r':
				return this->r;
			case 'o':
				return this->o;
			case 'O':
				return this->O;
			case 's':
				return this->s;
			default:
				return false;
		}
	}
	void	setMode( char mode, bool value )
	{
		switch (mode)
		{
			case 'a':
				this->a = value;
				break;
			case 'i':
				this->i = value;
				break;
			case 'w':
				this->w = value;
				break;
			case 'r':
				this->r = value;
				break;
			case 'o':
				this->o = value;
				break;
			case 'O':
				this->O = value;
				break;
			case 's':
				this->s = value;
				break;
			default:
				break;
		}
	}
}				t_modes;

std::string formatNumber( int number ) 
{
	std::ostringstream	oss;
	oss << std::setw(3) << std::setfill('0') << number;
	return oss.str();
}

std::string getTimestamp( void ) 
{
	std::string timestamp = "@time=";
	std::time_t t = std::time(0);
	std::tm *now = std::localtime(&t);

	std::ostringstream	oss;
	oss << (now->tm_year + 1900) << '-'
		<< std::setw(2) << std::setfill('0') << (now->tm_mon + 1) << '-'
		<< std::setw(2) << std::setfill('0') << now->tm_mday << 'T'
		<< std::setw(2) << std::setfill('0') << now->tm_hour << ':'
		<< std::setw(2) << std::setfill('0') << now->tm_min << ':'
		<< std::setw(2) << std::setfill('0') << now->tm_sec << "Z";

	timestamp += oss.str();
	return timestamp;
}

t_message	reply( int number, std::string message, std::vector<std::string> params ) 
{
	t_message	reply;
	reply.command = formatNumber(number);
	reply.prefix = ":" + server_name;
	std::string param = message;  // No ":"!! Hardcode that into the macro (be on the lookout for exceptions)

	size_t start = param.find('<');
	size_t end = param.find('>');
	for (size_t i = 0; i < params.size(); ++i) 
	{
		if (start != std::string::npos && end != std::string::npos) 
		{
			param.replace(start, end - start + 1, params[i]);
		}
		start = param.find('<');
		end = param.find('>');
	}

	reply.params.push_back(param);
	return reply;
}

t_message	reply( t_message & message, std::string corrected_param, std::string nickname ) 
{
	t_message	reply;
	reply.prefix = ":" + nickname;
	reply.command = message.command;
	std::string param = corrected_param; // TO-DO: Figure out the ":" part
	reply.params.push_back(param);
	return reply;
}

std::string get_mode_string( t_modes & modes )
{
	std::string mode_string = "+";
	if (modes.a)
		mode_string += "a";
	if (modes.i)
		mode_string += "i";
	if (modes.w)
		mode_string += "w";
	if (modes.r)
		mode_string += "r";
	if (modes.o)
		mode_string += "o";
	if (modes.O)
		mode_string += "O";
	if (modes.s)
		mode_string += "s";
	return mode_string;
}

bool isUserInServer( std::string nickname ) 
{
	return std::find(nicks_in_channel.begin(), nicks_in_channel.end(), nickname) != nicks_in_channel.end();
}

std::vector<t_message> cmdChanMode( t_message & message, t_modes modes ) 
{
	std::vector<t_message> replies;

	std::cout << "Channel mode part, not implemented yet" << std::endl;
	// Some modes take extra parameters, like +k <key> or +l <limit>
	// Note that there is a maximum limit of 3 changes per command for modes that require parameters
	// Unsure what this is saying exactly

	if (message.params.size() == 1) 
	{
		if (message.params[0][0] == '+') 
		{
			replies.push_back(reply(ERR_NOCHANMODES, ERR_NOCHANMODES_STR, { message.params[0] }));
			return replies;
		}
		replies.push_back(reply(RPL_CHANNELMODEIS, RPL_CHANNELMODEIS_STR, { message.params[0], get_mode_string(modes), /*get_mode_params_string(modes)*/ })); // TO-DO
		return replies;
	}

	/*
	Replies:
		ERR_KEYSET
			If the channel already has a key and the user tries to set it again
		ERR_CHANOPRIVSNEEDED
			If the user tries to set a mode that requires channel operator privileges
		ERR_USERNOTINCHANNEL
			If the mode to be set specifies as parameter a user, and that user is not in the channel
			i.e. +o <nickname>, trying to set as operator someone who doesn't belong to the channel
		ERR_UNKNOWNMODE
			If the mode is either invalid or not supported by the channel in some way
			i.e. +k <key>, on a channel that doesn't support keys
			or +X, since it's not a valid mode period
	*/

	// Provisional:
	std::vector<char> valid_modes;
	std::string correct_param;
	bool has_effect = false;
	bool operation = true;
	bool insert_operator = false;
	bool first_operator = true;
	bool unknown_flag_set = false;
	std::string param = message.params[1];
	for (size_t i = 0; i < param.size(); ++i) 
	{
		if (param[i] == '+') 
		{
			if (operation == false || first_operator == true)
				insert_operator = true;
			operation = true;
			first_operator = false;
		} else if (param[i] == '-') 
		{
			if (operation == true)
				insert_operator = true;
			operation = false;
			first_operator = false;
		} else 
		{
			char mode = param[i];
			if (std::string(USER_MODES).find(mode) == std::string::npos) 
			{
				if (unknown_flag_set == false) 
				{
					replies.push_back(reply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR, {}));
					unknown_flag_set = true;
				}
				continue;
			}
			bool has_mode = modes.getMode(mode);
			std::cout << "has mode " << mode << " ? " << has_mode << std::endl;
			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) 
			{
				if (operation == true && (mode == 'o' || mode == 'O'))
					continue;
				if (operation == false && mode == 'r')
					continue;
				if (mode == 'a')
					continue;
				modes.setMode(mode, operation);
				has_effect = true;
				if (insert_operator == true) 
				{
					valid_modes.push_back((operation ? '+' : '-') );
					insert_operator = false;
				}
				valid_modes.push_back(mode);
			}
		}
	}
	bool implicit_plus = false;
	if (!valid_modes.empty()) 
	{
		for (size_t i = 0; i < valid_modes.size(); ++i) 
		{
			if (i == 0 && valid_modes[i] != '+' && valid_modes[i] != '-') 
			{
				correct_param += '+';
				implicit_plus = true;
			}
			if (valid_modes[i] == '+' && implicit_plus == true) 
			{
				implicit_plus = false;
				continue;
			}
			correct_param += valid_modes[i];
		}
	}
	if (has_effect)
		replies.push_back(reply(message, correct_param, client_nickname));
	return replies;


	return replies;
}

std::vector<t_message> cmdMode( t_message & message, t_modes modes ) 
{
	std::vector<t_message>	replies;
	if (message.params.size() < 1) 
	{
		replies.push_back(reply(ERR_NEEDMOREPARAMS, ERR_NEEDMOREPARAMS_STR, { client_nickname }));
		return replies;
	}

	if (client_nickname != message.params[0]) 
	{
		if (isUserInServer(message.params[0]) == true)
			replies.push_back(reply(ERR_USERSDONTMATCH, ERR_USERSDONTMATCH_STR, {}));
		else if (std::find(channels.begin(), channels.end(), message.params[0]) != channels.end()) 
			return cmdChanMode(message, modes);
		else
			replies.push_back(reply(ERR_NOSUCHCHANNEL, ERR_NOSUCHCHANNEL_STR, { message.params[0] }));
		return replies;
	}

	if (message.params.size() == 1)
		replies.push_back(reply(RPL_UMODEIS, RPL_UMODEIS_STR, { get_mode_string(modes) }));

	std::vector<char> valid_modes;
	std::string correct_param;
	bool has_effect = false;
	bool operation = true;
	bool insert_operator = false;
	bool first_operator = true;
	bool unknown_flag_set = false;
	std::string param = message.params[1];
	for (size_t i = 0; i < param.size(); ++i) 
	{
		if (param[i] == '+') 
		{
			if (operation == false || first_operator == true)
				insert_operator = true;
			operation = true;
			first_operator = false;
		} else if (param[i] == '-') 
		{
			if (operation == true)
				insert_operator = true;
			operation = false;
			first_operator = false;
		} else 
		{
			char mode = param[i];
			if (std::string(USER_MODES).find(mode) == std::string::npos) 
			{
				if (unknown_flag_set == false) 
				{
					replies.push_back(reply(ERR_UMODEUNKNOWNFLAG, ERR_UMODEUNKNOWNFLAG_STR, {}));
					unknown_flag_set = true;
				}
				continue;
			}
			bool has_mode = modes.getMode(mode);
			std::cout << "has mode " << mode << " ? " << has_mode << std::endl;
			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) 
			{
				if (operation == true && (mode == 'o' || mode == 'O'))
					continue;
				if (operation == false && mode == 'r')
					continue;
				if (mode == 'a')
					continue;
				modes.setMode(mode, operation);
				has_effect = true;
				if (insert_operator == true) 
				{
					valid_modes.push_back((operation ? '+' : '-') );
					insert_operator = false;
				}
				valid_modes.push_back(mode);
			}
		}
	}
	bool implicit_plus = false;
	if (!valid_modes.empty()) {
		for (size_t i = 0; i < valid_modes.size(); ++i) 
		{
			if (i == 0 && valid_modes[i] != '+' && valid_modes[i] != '-') 
			{
				correct_param += '+';
				implicit_plus = true;
			}
			if (valid_modes[i] == '+' && implicit_plus == true) 
			{
				implicit_plus = false;
				continue;
			}
			correct_param += valid_modes[i];
		}
	}
	if (has_effect)
		replies.push_back(reply(message, correct_param, client_nickname));
	return replies;
}

// std::vector<t_message> mode_response( std::string param, t_modes modes )
// {
// 	std::vector<t_message> response;
// 	std::vector<char> valid_modes;

// 	bool operation = true;
// 	bool insert_operator = false;
// 	bool first_operator = true;
// 	for (int i = 0; i < param.size(); ++i) {
// 		if (param[i] == '+') {
// 			if (operation == false || first_operator == true)
// 				insert_operator = true;
// 			operation = true;
// 			first_operator = false;
// 		} else if (param[i] == '-') {
// 			if (operation == true)
// 				insert_operator = true;
// 			operation = false;
// 			first_operator = false;
// 		} else {
// 			char mode = param[i];
// 			// Verify that mode is contained in this str "aiwroOs"
// 			if (std::string("aiwroOs").find(mode) == std::string::npos)
// 				response.push_back({ "ERR_UMODEUNKNOWNFLAG", {} });
// 			bool has_mode = modes.getMode(mode);
// 			std::cout << "has mode " << mode << " ? " << has_mode << std::endl;
// 			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) {
// 				if (operation == true && (mode == 'o' || mode == 'O'))
// 					continue;
// 				if (operation == false && mode == 'r')
// 					continue;
// 			}
// 			modes.setMode(mode, operation);
// 			if (insert_operator == true) {
// 				valid_modes.push_back((operation ? '+' : '-') );
// 				insert_operator = false;
// 			}
// 			valid_modes.push_back(mode);
// 		}
// 	}
// 	if (!valid_modes.empty()) {
// 		for (int i = 0; i < valid_modes.size(); ++i) {
// 			response += valid_modes[i];
// 		}
// 	}
// 	return response;
// }

void assert_replies( std::vector<t_message> replies, int number, int expected_number, std::string expected_prefix, std::string expected_message, std::vector<std::string> expected_params ) 
{
	assert(replies.size() == static_cast<size_t>(number));
	assert(replies[0].prefix == expected_prefix);
	assert(replies[0].command == formatNumber(expected_number));
	assert(replies[0].params[0] == expected_message);
	for (size_t i = 0; i < expected_params.size(); ++i) 
	{
		assert(replies[0].params[i] == expected_params[i]);
	}
}

void print_replies( std::vector<t_message> replies ) 
{
	std::ostringstream ss;

	for (size_t i = 0; i < replies.size(); ++i) 
	{
		ss << ">> " << replies[i].prefix << " " << replies[i].command << " ";
		for (size_t j = 0; j < replies[i].params.size(); ++j) 
		{
			ss << replies[i].params[j] << " ";
		}
		ss << std::endl;
		std::cout << ss.str();
		ss.str("");
	}
}

void print_message( t_message & message ) 
{
	std::ostringstream ss;
	ss << "<< ";
	if (!message.prefix.empty())
		ss << message.prefix << " ";
	ss << message.command << " ";
	for (size_t i = 0; i < message.params.size(); ++i) 
	{
		ss << message.params[i];
		if (i < message.params.size() - 1)
			ss << " ";
	}
	ss << std::endl;
	std::cout << ss.str();
}

void clear_data( std::vector<t_message> & replies, t_message & message ) 
{
	replies.clear();
	message.params.clear();
}

void testCmdMode() 
{
	t_modes modes;
	modes.a = false;
	modes.i = false;
	modes.w = false;
	modes.r = false;
	modes.o = false;
	modes.O = false;
	modes.s = false;

	// Test case 1: No parameters
	std::cout << "Test 1: No parameters" << std::endl;
	t_message message;
	message.command = "MODE";
	message.params = {};
	std::vector<t_message> replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	std::cout << "Test 2: Nickname given doesn't match sender" << std::endl;
	// Test case 2: User doesn't match
	message.params = { "wrong_nickname", "+i" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	std::cout << "Test 2.1: Nickname given doesn't match sender, but matches other nickname in server" << std::endl;
	message.params = { "ffornes", "+i" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	// Test case 3: Invalid mode flag
	std::cout << "Test 3: Invalid mode flag" << std::endl;
	message.params = { client_nickname, "+K" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	std::cout << "Test 3.1: Invalid mode flags" << std::endl;
	message.params = { client_nickname, "+K+Q+Y+R+Z+Hola+305ha3ha0e--++" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	// Test case 4: Valid mode change
	std::cout << "Test 4.1: Valid mode change add" << std::endl;
	message.params = { client_nickname, "+i" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	modes.i = true;
	std::cout << "Test 4.2: Valid mode change remove" << std::endl;
	message.params = { client_nickname, "-i" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	std::cout << "Test 4.3: Valid mode change add and remove" << std::endl;
	message.params = { client_nickname, "-i+w" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;
	modes.i = false;

	// Test case 6: Multiple modes
	std::cout << "Test 6: Multiple modes" << std::endl;
	message.params = { client_nickname, "+iw" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	// Test case 7: Implicit '+' operator
	std::cout << "Test 7.1: Implicit '+' operator" << std::endl;
	message.params = { client_nickname, "iw" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	modes.w = true;
	std::cout << "Test 7.2: Implicit '+' operator" << std::endl;
	message.params = { client_nickname, "i-w" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;
	modes.w = false;

	// Test 8: Chaos
	std::cout << "Test 8.1: Chaos" << std::endl;
	message.params = { client_nickname, "+Q+K+w+i" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	// Test 9: Chaos 2
	std::cout << "Test 8.2: Chaos 2" << std::endl;
	message.params = { client_nickname, "+i++++w-o" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	std::cout << "Test 8.2: Chaos 3" << std::endl;
	message.params = { client_nickname, "i++++w-o+X" };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);
	std::cout << std::endl;

	std::cout << "Test 10: Channel mode" << std::endl;
	message.params = { "#lobby", };
	replies = cmdMode(message, modes);
	print_message(message);
	print_replies(replies);
	clear_data(replies, message);

	std::cout << "All tests passed!" << std::endl;
}

int main() 
{
	testCmdMode();
	return 0;
}

////////////////////// OLD MODE VERSION:
/*
Command: MODE
Parameters: <nickname> *( ( "+" / "-" ) *( "i" / "w" / "o" / "O" / "r" ) )
The User's mode typically changes how the client is seen by others or what 'extra'
messages the client is sent.
A MODE command is ONLY accepted if the nickname of the sender of the message and
the <nickname> parameter are the same.
If only <nickname> is provided, the server will return the current settings
for the nick.

It will receive all the modes in a single parameter, for example:
If more parameters are sent, they will be silently ignored.

i.e.:
+Q +i +w
This will be treated as +Q

+Qiw
This will be treated as +Q +i +w

This is also valid:

+Q+i+w in a single parameter

Also, things like +o are ignored, because a non op user cannot op themselves
From what I've seen in an example, this is the expected behavior:

<< MODE nick +o
(NO REPLY)

<< MODE nick +io
>> @timestamp :nick MODE nick :+i

So, the "o" part is ignored even from the acknoledge message

Now, in the params, you can have valid modes and invalid modes
If user modes are "" (nothing set)
And the user sends "+Q+K+w+i"


Q (in some servers) is valid, so "+Q" will be added
K is not a valid mode, so "+K" will trigger ERR_UMODEUNKNOWNFLAG
+w is valid, so "+w" will be added
and finally "+i" is valid, so "+i" will be added

SO, this will actually send two replies:

1. ERR_UMODEUNKNOWNFLAG reply
2. RPL_ACKNOWLEDGE reply

The acknowledge reply will have "+Q+w+i"

This is very annoying, we will have to think of how to handle multiple replies, etc.

Numeric Replies:
	ERR_NEEDMOREPARAMS
	ERR_USERSDONTMATCH
	ERR_UMODEUNKNOWNFLAG
	RPL_UMODEIS

//// VERY IMPORTANT ////
MODE is a dual-purpose command in the IRC protocol.
It allows both USERS and CHANNELS to change their modes.

For now, this only handles the user mode part

//
When parsing MODE messages, it is RECOMMENDED that the entire message be parsed
first, and then the changes be applied.

It is REQUIRED that the servers are able to change channel modes so that
"channel creator" and "channel operator" may be created. // idk yet
*/
// int Server::cmdMode( t_message & message ) {
// 	if (message.params.size() < 1)
// 		return ERR_NEEDMOREPARAMS;
// 	Client & client = this->_clients[message.sender_client_fd];

// 	if (client.getNickname() != message.params[0])
// 		return ERR_USERSDONTMATCH;

// 	if (message.params.size() == 1) {
// 		// This must handle the situation where the client wants to know their own modes
// 	}

// 	// Parse the parameter[0]
// 	std::string modes = message.params[1];
// 	bool operation = true; // true for add, false for remove
// 	for (int i = 0; i < modes.size(); ++i) {
// 		if (modes[i] == '+') {
// 			operation = true;
// 			continue;
// 		} else if (modes[i] == '-') {
// 			operation = false;
// 			continue;
// 		}
// 		switch (modes[i]) {
// 			case 'a':
// 				// A user cannot toggle the away mode with MODES, only with AWAY
// 				// Should this be handled in some way? TO-DO
// 				break;
// 			case 'i':
// 				client.setMode('i', operation);
// 				break;
// 			case 'w':
// 				client.setMode('w', operation);
// 				break;
// 			case 'o':
// 				if (operation == true)
// 				// +o is not allowed "The attempt should be IGNORED"
// 				if (operation == false) {
// 					// Maybe we should issue a warning, but technically nothing prevents you from deopping yourself
// 					client.setMode('o', operation);
// 				}
// 				break;
// 			case 'O':
// 				// +O is not allowed "The attempt should be IGNORED"
// 				if (operation == false)
// 					client.setMode('O', operation);
// 					// Maybe we should issue a warning, but technically nothing prevents you from deopping yourself
// 				break;
// 			case 'r':
// 				// -r is not allowed "The attempt should be IGNORED"
// 				client.setMode('r', operation);
// 				break;
// 			default:
// 				return ERR_UMODEUNKNOWNFLAG;
// 		}
// 	}

// 	// prepare the message for the acknowledge reply

// 	std::string param = message.params[0];
// 	std::string response;
// 	std::vector<char> valid_modes;

// 	bool operation = true;
// 	bool insert_operator = false;
// 	bool first_operator = true;
// 	for (int i = 0; i < param.size(); ++i) {
// 		if (param[i] == '+') {
// 			if (first_operator == true && operation == false)
// 				insert_operator = true;
// 			operation = true;
// 		} else if (param[i] == '-') {
// 			if (operation == true)
// 				insert_operator = true;
// 			operation = false;
// 		} else {
// 			char mode = param[i];
// 			bool has_mode = client.getMode(mode);
// 			if ((operation == true && has_mode == false) || (operation == false && has_mode == true)) {
// 				if (operation == true && (mode == 'o' || mode == 'O'))
// 					continue;
// 				if (operation == false && mode == 'r')
// 					continue;
// 			}
// 			client.setMode(mode, operation);
// 			if (insert_operator == true) {
// 				valid_modes.push_back((operation ? '+' : '-') );
// 				insert_operator = false;
// 			}
// 			valid_modes.push_back(mode);
// 		}
// 	}
// 	if (!valid_modes.empty()) {
// 		for (int i = 0; i < valid_modes.size(); ++i) {
// 			response += valid_modes[i];
// 		}
// 	}

// 	// In loop
// 	// 1. Seek next operator '+' or '-'
// 	// 2. Iterate through modes until next operator
// 	// 3. If no modes were found, remove the operator
// 	//


// 	return RPL_ACKNOWLEDGE;
// }
