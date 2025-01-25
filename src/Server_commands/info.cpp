#include "Server.hpp"

/*
    Command: INFO
    Parameters: None

    The INFO command provides general information about the server and its contributors.
    The response will be structured to show:
    - Server info
    - Contributors to the server (both code and ideas/testing)
    - Additional information like birth date and online time.
*/

std::vector<t_message> Server::cmdInfo(t_message &message) {
    std::cout << "INFO command called..." << std::endl;
    std::vector<t_message> replies;

    // Get the client who sent the command
    Client *client = findClient(message.sender_client_fd);
    if (client == NULL || !client->isRegistered()) {
        replies.push_back(createReply(ERR_NOTREGISTERED, ERR_NOTREGISTERED_STR));
        return replies;
    }

    // Server information for ft_irc
    std::vector<std::string> infoLines;
    infoLines.push_back("- ft_irc, a 42Barcelona project--");
    infoLines.push_back("- Based on the original code written by the authors of irssi: Timo Sirainen, Jarkko Hietaniemi, and others.");
    infoLines.push_back("- Copyright 2025 ft_irc team");
    infoLines.push_back("- ");
    infoLines.push_back("- This program is free software; you can redistribute it and/or");
    infoLines.push_back("- modify it under the terms of the GNU General Public License as");
    infoLines.push_back("- published by the Free Software Foundation; either version 1, or");
    infoLines.push_back("- (at your option) any later version.");
    infoLines.push_back("- ");
    infoLines.push_back("- Regular expression support is provided by the PCRE library package,");
    infoLines.push_back("- which is open source software, written by Philip Hazel, and copyright");
    infoLines.push_back("- by the University of Cambridge, England.");
    infoLines.push_back("- ");
    infoLines.push_back("- The following people currently compose the ft_irc team and");
    infoLines.push_back("- actively maintain the ft_irc codebase:");
    infoLines.push_back("- apresas https://github.com/apresas-97/");
    infoLines.push_back("- ffornes https://github.com/ffornesp");
    infoLines.push_back("- aaespino https://github.com/spnzed");
    infoLines.push_back("- ");
    infoLines.push_back("- Thanks to the following contributors for their code and support:");
    infoLines.push_back("- Okto dame mas dias");
    infoLines.push_back("- ");
    infoLines.push_back("- Thanks to those who have provided testing and feedback.");
    infoLines.push_back("- ");
    infoLines.push_back("- Birth Date: Fri Jan 25 2025 at 00:00:00 UTC, compile #1");
    infoLines.push_back("- On-line since Fri Jan 25 00:01:00 2025");
    infoLines.push_back("- End of /INFO list.");

    // Send each line as a response
    std::vector<std::string>::iterator it;
    for (it = infoLines.begin(); it != infoLines.end(); ++it) {
        std::vector<std::string> replyParams;
        replyParams.push_back(*it);
        t_message infoReply = createReply(RPL_INFO, RPL_INFO_STR, replyParams);
        replies.push_back(infoReply);
    }

    return replies;
}
