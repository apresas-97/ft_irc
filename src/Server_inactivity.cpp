#include "Server.hpp"

void Server::checkInactivity( void )
{
	if (this->_client_count == 0)
		return ;
	for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
	{
		Client & client = it->second;
		if (client.isTerminate())
			continue;
		if (client.isRegistered())
		{
			if (!client.isExpectedPong() && client.getLastActivity() + CLIENT_TIMEOUT_SECONDS < std::time(NULL))
			{
				t_message ping_message;
				ping_message.prefix = ":" + this->getName(); // I've seen servers not sending prefix for PING messages
				ping_message.command = "PING";
				ping_message.params.push_back(this->getName());
				ping_message.target_client_fds.insert(it->first);
				sendReply(ping_message);
				client.setExpectedPong(true);
				client.setPongTimer();
			}
			if (client.isExpectedPong() && client.getPongTimer() + CLIENT_PING_TIMEOUT_SECONDS < std::time(NULL))
			{
				this->_current_client = &client;
				t_message quit_message;
				quit_message.prefix = ":" + client.getUserPrefix();
				quit_message.command = "QUIT";
				quit_message.params.push_back("Ping timeout");
				std::vector<t_message> replies = this->cmdQuit(quit_message);
				for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
					sendReply(*it);
			}
		}
		else if (client.getLastActivity() + CLIENT_REGISTRATION_TIMEOUT_SECONDS < std::time(NULL))
		{
			this->_current_client = &client;
			client.setHostname("0.0.0.0");
			t_message quit_message;
			quit_message.prefix = ":" + client.getUserPrefix();
			quit_message.command = "QUIT";
			quit_message.params.push_back("Ping timeout");
			std::vector<t_message> replies = this->cmdQuit(quit_message);
			for (std::vector<t_message>::iterator it = replies.begin(); it != replies.end(); ++it)
				sendReply(*it);
		}
	}
}
