#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <sstream>

std::vector<std::string>	splitMessage( std::string message )
{
	std::vector<std::string>	split;
	std::string					word;
	std::istringstream			iss(message);

	while (iss >> word) {
		if (word.front() == ':' && split.size() > 0) {
			std::string rest;
			std::getline(iss, rest);
			word += rest;
			split.push_back(word);
			break;
		}
		split.push_back(word);
	}

	if (iss.bad()) {
		std::cerr << "Error reading from the input stream." << std::endl;
	}

	if (split.empty())
		std::cerr << "Empty messages should be silently ignored" << std::endl;

	return split;
}

void testSplitMessage() {
	// Test case 1: Simple message with command and arguments
	std::string message1 = "PRIVMSG #channel :Hello channel!";
	std::vector<std::string> result1 = splitMessage(message1);
	assert(result1.size() == 3);
	assert(result1[0] == "PRIVMSG");
	assert(result1[1] == "#channel");
	std::cout << result1[2] << std::endl;
	assert(result1[2] == ":Hello channel!");

	// Test case 2: Message with multiple spaces
	std::string message2 = "       INFO			apresas 	PRIVMSG             #channel                     :Hello         channel!";
	std::vector<std::string> result2 = splitMessage(message2);
	assert(result2.size() == 3);
	assert(result2[0] == "PRIVMSG");
	assert(result2[1] == "#channel");
	assert(result2[2] == ":Hello         channel!");

	// Test case 3: Message with prefix
	std::string message3 = ":prefix PRIVMSG #channel :Hello channel! I am apresas and I am new to IRC! I hope we all get along! >:)";
	std::vector<std::string> result3 = splitMessage(message3);
	assert(result3.size() == 4);
	assert(result3[0] == ":prefix");
	assert(result3[1] == "PRIVMSG");
	assert(result3[2] == "#channel");
	assert(result3[3] == ":Hello channel! I am apresas and I am new to IRC! I hope we all get along! >:)");

	// Test case 4: Empty message
	std::string message4 = "";
	std::vector<std::string> result4 = splitMessage(message4);
	assert(result4.empty());

	// Test case 5: Message without spaces
	std::string message5 = "COMMAND";
	std::vector<std::string> result5 = splitMessage(message5);
	assert(result5.size() == 1);
	assert(result5[0] == "COMMAND");

	// Test case 6: Message with only spaces
	std::string message6 = "   ";
	std::vector<std::string> result6 = splitMessage(message6);
	assert(result6.empty());

	std::cout << "All tests passed!" << std::endl;
}

int main() {
	testSplitMessage();
	return 0;
}